#include "platform.h"
#include "perlin.h"
#include "vector.h"
#include "bitmap.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_IMPLEMENTATION
#include "nuklear.h"
#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_nuklear.h"
#include "sokol_args.h"
#include "maths.h"
#include "default2d.glsl.h"
#include "default3d.glsl.h"
#if !WEB_BUILD
#include "filesystem.h"
#include "lua.h"
#define DMON_IMPL
#include "dmon.h"
#define THREADS_IMPL
#include "threads.h"
#define JIM_IMPLEMENTATION
#include "jim.h"
#define MJSON_IMPLEMENTATION
#include "mjson.h"
#define OSDIALOG_IMPLEMENTATION
#include "osdialog.h"
#endif

#define DEFAULT_CANVAS_SIZE 512

#define SETTINGS                              \
    X(int, canvasWidth, DEFAULT_CANVAS_SIZE)  \
    X(int, canvasHeight, DEFAULT_CANVAS_SIZE) \
    X(float, xoff, 0.f)                       \
    X(float, yoff, 0.f)                       \
    X(float, zoff, 0.f)                       \
    X(float, scale, 200.f)                    \
    X(float, lacunarity, 2.f)                 \
    X(float, gain, .5f)                       \
    X(int, octaves, 8)
typedef struct {
#define X(TYPE, NAME, DEFAULT) TYPE NAME;
    SETTINGS
#undef X
} Settings;

static Settings settings = {
#define X(TYPE, NAME, DEFAULT) .NAME = DEFAULT,
    SETTINGS
#undef X
};

typedef struct {
    Vec4 color;
    float max;
    int index;
} BiomeData;

typedef struct biome {
    BiomeData data;
    struct biome *next;
} Biome;

typedef struct {
    int count, tally;
    Biome *head, *tail;
} BiomeTree;

static struct {
    sg_pass_action pass_action;
    Vertex vertices[6];
    Bitmap bitmap;
    Texture texture;
    float delta;
    bool update;
    bool dragging;
    Vec2 lastMousePos, mousePos;
    int enableBiomes;
    BiomeTree biomes;
    struct {
        float zoom;
        Vec2 position;
        sg_pipeline pipeline;
        sg_bindings binding;
    } camera2d;
    struct {
        Vec3 position, target, up;
        sg_pipeline pipeline;
        sg_bindings binding;
    } camera3d;
    float scrollY;
#if !WEB_BUILD
    const char **models;
    int currentModel;
    const char **scripts;
    int currentScript;
    lua_State *luaState;
    mtx_t luaStateLock;
#endif
} state;

#if !WEB_BUILD
int LuaSettings(lua_State *L) {
    const char *setting = luaL_checkstring(L, 1);
    
    if (!lua_isnumber(L, 2)) {
#define X(TYPE, NAME, DEFAULT)                            \
        if (!strcmp(setting, #NAME)) {                    \
            lua_pushnumber(L, (lua_Number)settings.NAME); \
            state.update = true;                          \
            return 1;                                     \
        }
        SETTINGS
#undef X
        luaL_error(L, "Unknown setting: '%s'", setting);
        return 0;
    }
#define X(TYPE, NAME, DEFAULT)                        \
    if (!strcmp(setting, #NAME)) {                    \
        settings.NAME = (TYPE)luaL_checknumber(L, 2); \
        return 0;                                     \
    }
    SETTINGS
#undef X
    luaL_error(L, "Unknown setting: '%s'", setting);
    return 0;
}

int LuaDelta(lua_State *L) {
    lua_pushnumber(L, (lua_Number)state.delta);
    return 1;
}

static void WatchCallback(dmon_watch_id watch_id, dmon_action action, const char *dirname, const char *filename, const char *oldname, void *user) {
    const char *ext = FileExt(filename);
    if (!ext)
        return;
    char full[1024];
    sprintf(full, "%s%s", dirname, filename);
    
    switch (action) {
        case DMON_ACTION_CREATE:;
            if (!DoesFileExist(full))
                return;
            bool fallthrough = false;
            if (!strncmp("lua", ext, 3)) {
                bool alreadyExisted = false;
                for (int i = 0; i < VectorCount(state.scripts); i++)
                    if (!strcmp(filename, state.scripts[i])) {
                        alreadyExisted = true;
                        break;
                    }
                if (alreadyExisted)
                    fallthrough = true;
                else
                    VectorAppend(state.scripts, strdup(filename));
            }
            if (!fallthrough)
                break;
        case DMON_ACTION_MODIFY:
            if (!DoesFileExist(full))
                return;
            if (!strncmp("lua", ext, 3)) {
                if (!state.currentScript)
                    return;
                if (!strcmp(filename, state.scripts[state.currentScript-1])) {
                    mtx_lock(&state.luaStateLock);
                    lua_close(state.luaState);
                    state.luaState = LoadLuaScript(state.scripts[state.currentScript-1]);
                    mtx_unlock(&state.luaStateLock);
                }
            }
            break;
        case DMON_ACTION_DELETE:
            if (!strncmp("lua", ext, 3)) {
                for (int i = 0; i < VectorCount(state.scripts); i++)
                    if (!strcmp(state.scripts[i], filename)) {
                        free((void*)state.scripts[i]);
                        VectorRemove(state.scripts, i);
                        if (state.currentScript - 1 == i) {
                            mtx_lock(&state.luaStateLock);
                            lua_close(state.luaState);
                            state.luaState = NULL;
                            state.currentScript = 0;
                            mtx_unlock(&state.luaStateLock);
                        }
                    }
            }
            break;
        case DMON_ACTION_MOVE:
            //! TODO: Handle DMON_ACTION_MOVE (DMON_ACTION_MOVE never triggers on Mac?)
            break;
    }
}
#endif

void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext()
    });
    
    snk_setup(&(snk_desc_t) {});
    
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .action=SG_ACTION_CLEAR, .value={.1f, .1f, .1f, 1.f} }
    };
    
#define X(TYPE, NAME, DEFAULT) \
    if (sargs_exists(#NAME)) \
        settings.NAME = (TYPE)atof(sargs_value(#NAME));
    SETTINGS
#undef X
    
    state.texture = NewTexture(settings.canvasWidth, settings.canvasHeight);
    state.bitmap = NewBitmap(settings.canvasWidth, settings.canvasHeight);
    state.update = true;
    state.camera2d.zoom = 1.f;
    state.camera2d.position = (Vec2){0.f, 0.f};
    state.dragging = false;
    state.lastMousePos = state.mousePos = (Vec2){0.f,0.f};
    state.enableBiomes = 0;
    
#if !WEB_BUILD
    state.models = FindFiles("obj");
    state.currentModel = 0;
    state.scripts = FindFiles("lua");
    state.currentScript = 0;
    state.luaState = NULL;
    mtx_init(&state.luaStateLock, mtx_plain);
    
    dmon_init();
    assert(DoesDirExist("assets"));
    dmon_watch("assets", WatchCallback, DMON_WATCHFLAGS_IGNORE_DIRECTORIES, NULL);
#endif
    
    state.camera2d.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .shader = sg_make_shader(default2d_program_shader_desc(sg_query_backend())),
        .layout = {
            .buffers[0].stride = sizeof(Vertex),
            .attrs = {
                [ATTR_default2d_vs_position].format=SG_VERTEXFORMAT_FLOAT4,
                [ATTR_default2d_vs_texcoord].format=SG_VERTEXFORMAT_FLOAT2
            }
        }
    });
    
    state.camera2d.binding = (sg_bindings) {
        .fs_images[SLOT_tex] = state.texture,
        .vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc) {
            .usage = SG_USAGE_STREAM,
            .size = 6 * sizeof(Vertex)
        })
    };
}


static void SwapBiomes(Biome *a, Biome *b) {
    BiomeData tmp;
    memcpy(&tmp, &a->data, sizeof(BiomeData));
    memcpy(&a->data, &b->data, sizeof(BiomeData));
    memcpy(&b->data, &tmp, sizeof(BiomeData));
}

static void SortBiomes(void) {
    Biome *cursor = state.biomes.head;
    Biome *tmp = NULL;
    while (cursor) {
        tmp = cursor->next;
        while (tmp) {
            if (cursor->data.max > tmp->data.max)
                SwapBiomes(cursor, tmp);
            tmp = tmp->next;
        }
        cursor = cursor->next;
    }
}

#define MAX_BIOMES 16

static void AddNewBiome(Vec4 color, float max) {
    if (state.biomes.count >= MAX_BIOMES)
        return;
    
    Biome *result = malloc(sizeof(Biome));
    result->data = (BiomeData) {
        .color = color,
        .max   = max,
        .index = ++state.biomes.tally
    };
    result->next  = NULL;
    
    state.biomes.count++;
    if (!state.biomes.head)
        state.biomes.head = state.biomes.tail = result;
    else
        state.biomes.tail = state.biomes.tail->next = result;
    
    SortBiomes();
}

static void RemoveBiome(Biome *biome) {
    Biome *cursor = state.biomes.head, *prev = NULL;
    while (cursor) {
        if (cursor->data.index == biome->data.index) {
            if (!prev)
                state.biomes.head = cursor->next;
            else
                prev->next = cursor->next;
            state.biomes.count--;
            free(cursor);
            break;
        }
        prev = cursor;
        cursor = cursor->next;
    }
}

static int ColorToRGB(Vec4 color) {
    return RGBA((int)(color.x * 255.f), (int)(color.y * 255.f), (int)(color.z * 255.f), (int)(color.w * 255));
}

static void DestroyBiomes(void) {
    Biome *cursor = state.biomes.head;
    while (cursor) {
        Biome *tmp = cursor->next;
        free(cursor);
        cursor = tmp;
    }
}

#if !WEB_BUILD
static void ExportBiomes(const char *path) {
    FILE *fh = fopen(path, "w");
    Jim jim = {
        .sink = fh,
        .write = (Jim_Write)fwrite
    };
    jim_object_begin(&jim);
    jim_member_key(&jim, "biomes");
    jim_array_begin(&jim);
    Biome *cursor = state.biomes.head;
    while (cursor) {
        jim_object_begin(&jim);
        jim_member_key(&jim, "r");
        jim_integer(&jim, (long long)(cursor->data.color.x * 255.f));
        jim_member_key(&jim, "g");
        jim_integer(&jim, (long long)(cursor->data.color.y * 255.f));
        jim_member_key(&jim, "b");
        jim_integer(&jim, (long long)(cursor->data.color.z * 255.f));
        jim_member_key(&jim, "a");
        jim_integer(&jim, (long long)(cursor->data.color.w * 255.f));
        jim_member_key(&jim, "max");
        jim_float(&jim, (double)cursor->data.max, 2);
        jim_object_end(&jim);
        cursor = cursor->next;
    }
    jim_array_end(&jim);
    jim_object_end(&jim);
    fclose(fh);
}

static void LoadBiomes(const char *path) {
    if (state.biomes.head)
        DestroyBiomes();
    
    int colorR[MAX_BIOMES];
    int colorG[MAX_BIOMES];
    int colorB[MAX_BIOMES];
    int colorA[MAX_BIOMES];
    double max[MAX_BIOMES];
    const struct json_attr_t biome_attr[] = {
        {"r", t_integer, .addr.integer=colorR},
        {"g", t_integer, .addr.integer=colorG},
        {"b", t_integer, .addr.integer=colorB},
        {"a", t_integer, .addr.integer=colorA},
        {"max", t_real, .addr.real=max},
        {NULL}
    };
    int biomeCount = 0;
    const struct json_attr_t root_attr[] = {
        {"biomes", t_array, .addr.array.element_type=t_object,
                            .addr.array.arr.objects.subtype=biome_attr,
                            .addr.array.maxlen=MAX_BIOMES,
                            .addr.array.count=&biomeCount},
        {NULL}
    };
    
    char *json = LoadFile(path, NULL);
    assert(json);
    int status = json_read_object(json, root_attr, NULL);
    assert(!status);
    assert(biomeCount);
    
    for (int i = 0; i < biomeCount; i++)
        AddNewBiome((Vec4){(float)colorR[i] / 255.f, (float)colorG[i] / 255.f, (float)colorB[i] / 255.f, (float)colorA[i] / 255.f}, max[i]);
}

static void ExportSettings(const char *path) {
    FILE *fh = fopen(path, "w");
    Jim jim = {
        .sink = fh,
        .write = (Jim_Write)fwrite
    };
    jim_object_begin(&jim);
    jim_member_key(&jim, "perlin");
    jim_object_begin(&jim);
#define X(TYPE, NAME, DEFAULT)   \
    jim_member_key(&jim, #NAME); \
    jim_float(&jim,  (float)settings.NAME, 2);
    SETTINGS
#undef X
    jim_object_end(&jim);
    jim_object_end(&jim);
    fclose(fh);
}

static void LoadSettings(const char *path, Settings *out) {
    struct {
#define X(TYPE, NAME, DEFAULT) double NAME;
        SETTINGS
#undef X
    } tmp;
    
    const struct json_attr_t settings_attr[] = {
#define X(TYPE, NAME, DEFAULT) { #NAME, t_real, .addr.real=&tmp.NAME },
        SETTINGS
#undef X
        {NULL}
    };
    const struct json_attr_t root_attr[] = {
        {"perlin", t_object, .addr.attrs=settings_attr},
        {NULL}
    };
    
    char *json = LoadFile(path, NULL);
    assert(json);
    int status = json_read_object(json, root_attr, NULL);
    assert(!status);
    
#define X(TYPE, NAME, DEFAULT) out->NAME = (TYPE)tmp.NAME;
    SETTINGS
#undef X
    
    free(json);
}
#endif

void frame(void) {
    state.delta = (float)(sapp_frame_duration() * 60.0);
    
#if !WEB_BUILD
    if (state.currentScript != 0) {
        mtx_lock(&state.luaStateLock);
        LuaCallPreframe(state.luaState);
        mtx_unlock(&state.luaStateLock);
    }
#endif
    
    struct nk_context *ctx = snk_new_frame();
    Settings tmp;
    memcpy(&tmp, &settings, sizeof(Settings));
   
#if !WEB_BUILD
    int currentModel = state.currentModel, currentScript = state.currentScript;
#endif
    bool resetValues = false;
    if (nk_begin(ctx, "Settings", nk_rect(0, 0, 300, (int)(300.f * PHI)), NK_WINDOW_SCALABLE | NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE)) {
        if (nk_tree_push(ctx, NK_TREE_TAB, "Size", NK_MINIMIZED)) {
            nk_property_int(ctx, "#Width:", 128, &tmp.canvasWidth, 1024, 16, 1);
            nk_property_int(ctx, "#Height:", 128, &tmp.canvasHeight, 1024, 16, 1);
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Noise", NK_MAXIMIZED)) {
            nk_property_float(ctx, "#X:", 0.f, &tmp.xoff, FLT_MAX, 1.f, 1);
            nk_property_float(ctx, "#Y:", 0.f, &tmp.yoff, FLT_MAX, 1.f, 1);
            nk_property_float(ctx, "#Z:", 0.f, &tmp.zoff, FLT_MAX, 1.f, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Scale: %f", settings.scale);
            nk_slider_float(ctx, .1f, &tmp.scale, 1024.f, .1f);
            nk_labelf(ctx, NK_TEXT_LEFT, "Lacunarity: %f", settings.lacunarity);
            nk_slider_float(ctx, .1f, &tmp.lacunarity, 16.f, .1f);
            nk_labelf(ctx, NK_TEXT_LEFT, "Gain: %f", settings.gain);
            nk_slider_float(ctx, .1f, &tmp.gain, 5.f, .1f);
            nk_labelf(ctx, NK_TEXT_LEFT, "Octaves: %d", settings.octaves);
            nk_slider_int(ctx, 1, &tmp.octaves, 16, 1);
            if (nk_button_label(ctx, "Reset"))
                resetValues = true;
#if !WEB_BUILD
            if (nk_button_label(ctx, "Import Settings")) {
                osdialog_filters *filters = osdialog_filters_parse("JSON:json");
                char *filename = osdialog_file(OSDIALOG_OPEN, ".", NULL, filters);
                if (filename)
                    LoadSettings(filename, &tmp);
                osdialog_filters_free(filters);
            }
            if (nk_button_label(ctx, "Export Settings")) {
                char path[256];
                time_t raw = time(NULL);
                struct tm *t = localtime(&raw);
                strftime(path, 256, "Perlin %G-%m-%d at %H.%M.%S.json", t);
                ExportSettings(path);
            }
#endif
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Biomes", NK_MINIMIZED)) {
            bool lastEnabled = state.enableBiomes;
            nk_checkbox_label(ctx, "Enable biomes", &state.enableBiomes);
            if (state.enableBiomes != lastEnabled)
                state.update = true;
            if (state.enableBiomes) {
                Biome *cursor = state.biomes.head;
                while (cursor) {
                    bool removed = false;
                    Vec4 lastColor = cursor->data.color;
                    float lastMax = cursor->data.max;
                    struct nk_colorf color = (struct nk_colorf){cursor->data.color.x,cursor->data.color.y,cursor->data.color.z,cursor->data.color.w};
                    if (nk_combo_begin_color(ctx, nk_rgba_cf(color), nk_vec2(200,400))) {
                        nk_layout_row_dynamic(ctx, 120, 1);
                        color = nk_color_picker(ctx, color, NK_RGBA);
                        
                        nk_layout_row_dynamic(ctx, 25, 1);
                        color.r = nk_propertyf(ctx, "#R:", 0, color.r, 1.f, .01f, .005f);
                        color.g = nk_propertyf(ctx, "#G:", 0, color.g, 1.f, .01f, .005f);
                        color.b = nk_propertyf(ctx, "#B:", 0, color.b, 1.f, .01f, .005f);
                        color.a = nk_propertyf(ctx, "#A:", 0, color.a, 1.f, .01f, .005f);
                        cursor->data.color = (Vec4){color.r, color.g, color.b, color.a};
                        cursor->data.max = nk_propertyf(ctx, "#Max:", 0, cursor->data.max, 1.f, .01f, .005f);
                        if (!Vec4Eq(lastColor, cursor->data.color) || lastMax != cursor->data.max)
                            state.update = true;
                        if (nk_button_label(ctx, "Remove Biome")) {
                            RemoveBiome(cursor);
                            removed = true;
                            state.update = true;
                            nk_combo_close(ctx);
                        }
                        nk_combo_end(ctx);
                    }
                    if (!removed)
                        cursor = cursor->next;
                }
                
                if (nk_button_label(ctx, "Add Biome")) {
                    AddNewBiome((Vec4){0.f,0.f,0.f,0.f}, 0.f);
                    state.update = true;
                }
#if !WEB_BUILD
                if (nk_button_label(ctx, "Import Biomes")) {
                    osdialog_filters *filters = osdialog_filters_parse("JSON:json");
                    char *filename = osdialog_file(OSDIALOG_OPEN, ".", NULL, filters);
                    if (filename)
                        LoadBiomes(filename);
                    osdialog_filters_free(filters);
                }
                if (nk_button_label(ctx, "Export Biomes") && state.biomes.head) {
                    char path[256];
                    time_t raw = time(NULL);
                    struct tm *t = localtime(&raw);
                    strftime(path, 256, "Biomes %G-%m-%d at %H.%M.%S.json", t);
                    ExportBiomes(path);
                }
#endif
            }
            nk_tree_pop(ctx);
        }
#if !WEB_BUILD
        if (nk_tree_push(ctx, NK_TREE_TAB, "Script", NK_MINIMIZED)) {
            int scriptCount = 1 + (state.scripts ? VectorCount(state.scripts) : 0);
            const char* defaultScripts[scriptCount];
            defaultScripts[0] = "Default (Nothing)";
            memcpy(defaultScripts + 1, state.scripts, VectorCount(state.scripts) * sizeof(const char*));
            currentScript = nk_combo(ctx, defaultScripts, scriptCount, currentScript, 20, nk_vec2(200, 200));
            nk_tree_pop(ctx);
        }
        if (nk_button_label(ctx, "Export")) {
            char path[256];
            time_t raw = time(NULL);
            struct tm *t = localtime(&raw);
            strftime(path, 256, "Perlin %G-%m-%d at %H.%M.%S.png", t);
            ExportBitmap(&state.bitmap, path);
        }
#endif
    }
    nk_end(ctx);
   
    if (!nk_window_is_any_hovered(ctx)) {
        state.camera2d.zoom = CLAMP(state.camera2d.zoom + (state.scrollY * state.delta), .1f, 10.f);
        
        if (state.dragging)
            state.camera2d.position -= state.lastMousePos - state.mousePos;
    }
    
#if !WEB_BUILD
    if (currentModel != state.currentModel) {
        state.update = true;
        state.currentModel = currentModel;
    }
    
    if (currentScript != state.currentScript) {
        mtx_lock(&state.luaStateLock);
        state.update = true;
        state.currentScript = currentScript;
        if (!currentScript) {
            if (state.luaState) {
                lua_close(state.luaState);
                state.luaState = NULL;
            }
        } else {
            if (state.luaState)
                lua_close(state.luaState);
            state.luaState = LoadLuaScript(state.scripts[state.currentScript-1]);
        }
        mtx_unlock(&state.luaStateLock);
    }
#endif
    
    if (resetValues) {
#define X(TYPE, NAME, DEFAULT) tmp.NAME = DEFAULT;
        SETTINGS
#undef X
        state.update = true;
    }
    
    if (tmp.canvasWidth != settings.canvasWidth || tmp.canvasHeight != settings.canvasHeight) {
        settings.canvasWidth = tmp.canvasWidth;
        settings.canvasHeight = tmp.canvasHeight;
        DestroyBitmap(&state.bitmap);
        state.bitmap = NewBitmap(settings.canvasWidth, settings.canvasHeight);
        DestroyTexture(state.texture);
        state.texture = NewTexture(settings.canvasWidth, settings.canvasHeight);
        state.camera2d.binding.fs_images[SLOT_tex] = state.texture;
        state.update = true;
    }
    
#define X(TYPE, NAME, DEFAULT)                      \
    if (!state.update && tmp.NAME != settings.NAME) \
        state.update = true;
    SETTINGS
#undef X
    
    if (state.update) {
        memcpy(&settings, &tmp, sizeof(Settings));
        unsigned char *heightmap = PerlinFBM(settings.canvasWidth, settings.canvasHeight, settings.zoff, settings.xoff, settings.yoff, settings.scale, settings.lacunarity, settings.gain, settings.octaves);
#if !WEB_BUILD
        if (state.currentScript != 0) {
            mtx_lock(&state.luaStateLock);
            LuaCallFrame(state.luaState, heightmap, settings.canvasWidth, settings.canvasHeight);
            mtx_unlock(&state.luaStateLock);
        }
#endif
        if (state.enableBiomes)
            SortBiomes();
        for (int x = 0; x < settings.canvasWidth; x++)
            for (int y = 0; y < settings.canvasHeight; y++) {
                int i = y * settings.canvasWidth + x;
                unsigned char h = heightmap[i];
                if (state.enableBiomes && state.biomes.head) {
                    Biome *cursor = state.biomes.head, *prev = NULL;
                    bool found = false;
                    while (cursor) {
                        if (h <= (unsigned char)(cursor->data.max * 255.f)) {
                            if (prev)
                                state.bitmap.buf[i] = ColorToRGB(cursor->data.color);
                            else
                                state.bitmap.buf[i] = ColorToRGB(state.biomes.head->data.color);
                            found = true;
                            break;
                        }
                        prev = cursor;
                        cursor = cursor->next;
                    }
                    if (!found)
                        state.bitmap.buf[i] = RGB(h, h, h);
                } else
                    state.bitmap.buf[i] = RGB(h, h, h);
            }
        
#if !WEB_BUILD
        if (state.currentScript != 0) {
            mtx_lock(&state.luaStateLock);
            LuaCallPostframe(state.luaState, &state.bitmap);
            mtx_unlock(&state.luaStateLock);
        }
#endif
        
        sg_update_image(state.texture, &(sg_image_data) {
            .subimage[0][0] = {
                .ptr  = state.bitmap.buf,
                .size = state.bitmap.w * state.bitmap.h * sizeof(int)
            }
        });
        state.update = false;
    }
    
    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.camera2d.pipeline);
    
    Vec2 size = {settings.canvasWidth, settings.canvasHeight};
    Vec2 viewport = {sapp_width(), sapp_height()};
    Vec2 position = state.camera2d.position + (viewport / 2.f) - (size / 2.f);
    Vec2 quad[4] = {
        {position.x, position.y + size.y}, // bottom left
        position + size, // bottom right
        {position.x + size.x, position.y }, // top right
        position, // top left
    };
    Vec2 v = (Vec2){2.f,-2.f} / viewport;
    for (int j = 0; j < 4; j++)
        quad[j] = (v * quad[j] + (Vec2){-1.f, 1.f}) * state.camera2d.zoom;
    
    static const Vec2 vtexquad[4] = {
        {0.f, 1.f}, // bottom left
        {1.f, 1.f}, // bottom right
        {1.f, 0.f}, // top right
        {0.f, 0.f}, // top left
    };
    static const int indices[6] = {
        0, 1, 2,
        3, 0, 2
    };
    for (int i = 0; i < 6; i++)
        state.vertices[i] = (Vertex) {
            .position = V2TOV4(quad[indices[i]]),
            .texcoord = vtexquad[indices[i]]
        };
    
    sg_update_buffer(state.camera2d.binding.vertex_buffers[0], &(sg_range) {
        .ptr = state.vertices,
        .size = 6 * sizeof(Vertex)
    });
    sg_apply_bindings(&state.camera2d.binding);
    sg_draw(0, 6, 1);
    
    snk_render(sapp_width(), sapp_height());
    sg_end_pass();
    
    sg_commit();
    state.scrollY = 0.f;
    state.lastMousePos = state.mousePos;
}

void event(const sapp_event *e) {
    snk_handle_event(e);
    switch (e->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
#if !WEB_BUILD
#if defined(PLATFORM_MAC)
            if (e->modifiers & SAPP_MODIFIER_SUPER && e->key_code == SAPP_KEYCODE_W)
                sapp_quit();
#else
            if (e->modifiers & SAPP_MODIFIER_ALT && e->key_code == SAPP_KEYCODE_F4)
                sapp_quit();
#endif
#endif
            break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            state.scrollY = e->scroll_y;
            break;
        case SAPP_EVENTTYPE_MOUSE_DOWN:
        case SAPP_EVENTTYPE_MOUSE_UP:
            state.dragging = e->mouse_button == SAPP_MOUSEBUTTON_LEFT && e->type == SAPP_EVENTTYPE_MOUSE_DOWN;
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            state.mousePos = (Vec2){e->mouse_x, e->mouse_y};
            break;
        default:
            break;
    }
}

void cleanup(void) {
#if !WEB_BUILD
    for (int i = 0; i < VectorCount(state.models); i++)
        free((void*)state.models[i]);
    DestroyVector(state.models);
    for (int i = 0; i < VectorCount(state.scripts); i++)
        free((void*)state.scripts[i]);
    DestroyVector(state.scripts);
    dmon_deinit();
#endif
    DestroyBiomes();
    DestroyBitmap(&state.bitmap);
    snk_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    sargs_setup(&(sargs_desc){ .argc=argc, .argv=argv });
#define CHECK_ARG_INT(NAME, DEFAULT) \
    int NAME = DEFAULT; \
    if (sargs_exists(#NAME)) { \
        int tmp = atoi(sargs_value(#NAME)); \
        if (tmp) \
            NAME = tmp; \
    }
    CHECK_ARG_INT(width, 1280);
    CHECK_ARG_INT(height, 720);
    CHECK_ARG_INT(samples, 4);
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = event,
        .cleanup_cb = cleanup,
        .width = width,
        .height = height,
        .sample_count = samples,
        .window_title = "perlin",
        .icon.sokol_default = true
    };
}
