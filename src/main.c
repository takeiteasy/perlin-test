#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_IMPLEMENTATION
#include "nuklear.h"
#include "bitmap.h"
#include "maths.h"
#include "perlin.h"
#include "vector.h"
#if !WEB_BUILD
#include "filesystem.h"
#include "lua.h"
#define DMON_IMPL
#include "dmon.h"
#include <time.h>
#endif
#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_nuklear.h"
#include "default.glsl.h"

#define DEFAULT_CANVAS_SIZE 512

#define SETTINGS \
    X(int, canvasWidth, DEFAULT_CANVAS_SIZE) \
    X(int, canvasHeight, DEFAULT_CANVAS_SIZE) \
    X(float, xoff, 0.f) \
    X(float, yoff, 0.f) \
    X(float, zoff, 0.f) \
    X(float, scale, 200.f) \
    X(float, lacunarity, 2.f) \
    X(float, gain, .5f) \
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

static struct {
    sg_pass_action pass_action;
    sg_pipeline pipeline;
    sg_bindings binding;
    Vertex vertices[6];
    Bitmap bitmap;
    Texture texture;
    float delta;
    bool update;
    float zoom;
    float scrollY;
#if !WEB_BUILD
    const char **models;
    int currentModel;
    const char **scripts;
    int currentScript;
    lua_State *luaState;
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

static void WatchCallback(dmon_watch_id watch_id, dmon_action action, const char* dirname, const char* filename, const char* oldname, void* user) {
    switch (action) {
        case DMON_ACTION_CREATE:
            break;
        case DMON_ACTION_DELETE:
            break;
        case DMON_ACTION_MODIFY:
            break;
        case DMON_ACTION_MOVE:
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
    
    state.texture = NewTexture(settings.canvasWidth, settings.canvasHeight);
    state.bitmap = NewBitmap(settings.canvasWidth, settings.canvasHeight);
    state.update = true;
    state.zoom = 1.f;
    
#if !WEB_BUILD
    state.models = FindFiles("obj");
    state.currentModel = 0;
    state.scripts = FindFiles("lua");
    state.currentScript = 0;
    state.luaState = NULL;
    
    dmon_init();
    dmon_watch("assets", WatchCallback, DMON_WATCHFLAGS_IGNORE_DIRECTORIES, NULL);
#endif
    
    state.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .shader = sg_make_shader(default_program_shader_desc(sg_query_backend())),
        .layout = {
            .buffers[0].stride = sizeof(Vertex),
            .attrs = {
                [ATTR_default_vs_position].format=SG_VERTEXFORMAT_FLOAT4,
                [ATTR_default_vs_texcoord].format=SG_VERTEXFORMAT_FLOAT2
            }
        }
    });
    
    state.binding = (sg_bindings) {
        .fs_images[SLOT_tex] = state.texture,
        .vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc) {
            .usage = SG_USAGE_STREAM,
            .size = 6 * sizeof(Vertex)
        })
    };
}

void frame(void) {
    state.delta = (float)(sapp_frame_duration() * 60.0);
    
#if !WEB_BUILD
    if (state.currentScript != 0)
        LuaCallPreframe(state.luaState);
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
        if (nk_tree_push(ctx, NK_TREE_TAB, "Target", NK_MINIMIZED)) {
            int modelCount = 2 + (state.models ? VectorCount(state.models) : 0);
            const char* defaultModels[modelCount];
            defaultModels[0] = "Default (2D)";
            defaultModels[1] = "Heightmap";
            memcpy(defaultModels + 2, state.models, VectorCount(state.models) * sizeof(const char*));
            currentModel = nk_combo(ctx, defaultModels, modelCount, currentModel, 20, nk_vec2(200, 200));
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
   
    if (!nk_window_is_any_hovered(ctx))
        state.zoom = CLAMP(state.zoom + (state.scrollY * state.delta), .1f, 10.f);
    
#if !WEB_BUILD
    if (currentModel != state.currentModel) {
        state.update = true;
        state.currentModel = currentModel;
    }
    
    if (currentScript != state.currentScript) {
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
        state.binding.fs_images[SLOT_tex] = state.texture;
        state.update = true;
    }
    
    if (tmp.xoff != settings.xoff ||
        tmp.yoff != settings.yoff ||
        tmp.zoff != settings.zoff ||
        tmp.scale != settings.scale ||
        tmp.lacunarity != settings.lacunarity ||
        tmp.gain != settings.gain ||
        tmp.octaves != settings.octaves)
        state.update = true;
    
    if (state.update) {
        memcpy(&settings, &tmp, sizeof(Settings));
        unsigned char *heightmap = PerlinFBM(settings.canvasWidth, settings.canvasHeight, settings.zoff, settings.xoff, settings.yoff, settings.scale, settings.lacunarity, settings.gain, settings.octaves);
        for (int x = 0; x < settings.canvasWidth; x++)
            for (int y = 0; y < settings.canvasHeight; y++) {
                int i = y * settings.canvasWidth + x;
                unsigned char h = heightmap[i];
                state.bitmap.buf[i] = RGB(h, h, h);
            }
        
#if !WEB_BUILD
        if (state.currentScript != 0)
            LuaCallFrame(state.luaState, &state.bitmap);
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
    sg_apply_pipeline(state.pipeline);
    
    Vec2 size = {settings.canvasWidth, settings.canvasHeight};
    Vec2 viewport = {sapp_width(), sapp_height()};
    Vec2 position = (viewport / 2.f) - (size / 2.f);
    Vec2 quad[4] = {
        {position.x, position.y + size.y}, // bottom left
        position + size, // bottom right
        {position.x + size.x, position.y }, // top right
        position, // top left
    };
    Vec2 v = (Vec2){2.f,-2.f} / viewport;
    for (int j = 0; j < 4; j++)
        quad[j] = (v * quad[j] + (Vec2){-1.f, 1.f}) * state.zoom;
    
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
    
    sg_update_buffer(state.binding.vertex_buffers[0], &(sg_range) {
        .ptr = state.vertices,
        .size = 6 * sizeof(Vertex)
    });
    sg_apply_bindings(&state.binding);
    
    sg_draw(0, 6, 1);
    snk_render(sapp_width(), sapp_height());
    sg_end_pass();
    
    sg_commit();
    state.scrollY = 0.f;
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
    DestroyBitmap(&state.bitmap);
    snk_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = event,
        .cleanup_cb = cleanup,
        .width = 1280,
        .height = 720,
        .sample_count = 4,
        .gl_force_gles2 = true,
        .window_title = "perlin",
        .icon.sokol_default = true,
    };
}
