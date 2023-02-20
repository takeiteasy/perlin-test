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
#include "default.glsl.h"
#include <float.h>

static const float grad3[][3] = {
    { 1, 1, 0 }, { -1, 1, 0 }, { 1, -1, 0 }, { -1, -1, 0 },
    { 1, 0, 1 }, { -1, 0, 1 }, { 1, 0, -1 }, { -1, 0, -1 },
    { 0, 1, 1 }, { 0, -1, 1 }, { 0, 1, -1 }, { 0, -1, -1 }
};

static const unsigned int perm[] = {
    182, 232, 51, 15, 55, 119, 7, 107, 230, 227, 6, 34, 216, 61, 183, 36,
    40, 134, 74, 45, 157, 78, 81, 114, 145, 9, 209, 189, 147, 58, 126, 0,
    240, 169, 228, 235, 67, 198, 72, 64, 88, 98, 129, 194, 99, 71, 30, 127,
    18, 150, 155, 179, 132, 62, 116, 200, 251, 178, 32, 140, 130, 139, 250, 26,
    151, 203, 106, 123, 53, 255, 75, 254, 86, 234, 223, 19, 199, 244, 241, 1,
    172, 70, 24, 97, 196, 10, 90, 246, 252, 68, 84, 161, 236, 205, 80, 91,
    233, 225, 164, 217, 239, 220, 20, 46, 204, 35, 31, 175, 154, 17, 133, 117,
    73, 224, 125, 65, 77, 173, 3, 2, 242, 221, 120, 218, 56, 190, 166, 11,
    138, 208, 231, 50, 135, 109, 213, 187, 152, 201, 47, 168, 185, 186, 167, 165,
    102, 153, 156, 49, 202, 69, 195, 92, 21, 229, 63, 104, 197, 136, 148, 94,
    171, 93, 59, 149, 23, 144, 160, 57, 76, 141, 96, 158, 163, 219, 237, 113,
    206, 181, 112, 111, 191, 137, 207, 215, 13, 83, 238, 249, 100, 131, 118, 243,
    162, 248, 43, 66, 226, 27, 211, 95, 214, 105, 108, 101, 170, 128, 210, 87,
    38, 44, 174, 188, 176, 39, 14, 143, 159, 16, 124, 222, 33, 247, 37, 245,
    8, 4, 22, 82, 110, 180, 184, 12, 25, 5, 193, 41, 85, 177, 192, 253,
    79, 29, 115, 103, 142, 146, 52, 48, 89, 54, 121, 212, 122, 60, 28, 42,
    
    182, 232, 51, 15, 55, 119, 7, 107, 230, 227, 6, 34, 216, 61, 183, 36,
    40, 134, 74, 45, 157, 78, 81, 114, 145, 9, 209, 189, 147, 58, 126, 0,
    240, 169, 228, 235, 67, 198, 72, 64, 88, 98, 129, 194, 99, 71, 30, 127,
    18, 150, 155, 179, 132, 62, 116, 200, 251, 178, 32, 140, 130, 139, 250, 26,
    151, 203, 106, 123, 53, 255, 75, 254, 86, 234, 223, 19, 199, 244, 241, 1,
    172, 70, 24, 97, 196, 10, 90, 246, 252, 68, 84, 161, 236, 205, 80, 91,
    233, 225, 164, 217, 239, 220, 20, 46, 204, 35, 31, 175, 154, 17, 133, 117,
    73, 224, 125, 65, 77, 173, 3, 2, 242, 221, 120, 218, 56, 190, 166, 11,
    138, 208, 231, 50, 135, 109, 213, 187, 152, 201, 47, 168, 185, 186, 167, 165,
    102, 153, 156, 49, 202, 69, 195, 92, 21, 229, 63, 104, 197, 136, 148, 94,
    171, 93, 59, 149, 23, 144, 160, 57, 76, 141, 96, 158, 163, 219, 237, 113,
    206, 181, 112, 111, 191, 137, 207, 215, 13, 83, 238, 249, 100, 131, 118, 243,
    162, 248, 43, 66, 226, 27, 211, 95, 214, 105, 108, 101, 170, 128, 210, 87,
    38, 44, 174, 188, 176, 39, 14, 143, 159, 16, 124, 222, 33, 247, 37, 245,
    8, 4, 22, 82, 110, 180, 184, 12, 25, 5, 193, 41, 85, 177, 192, 253,
    79, 29, 115, 103, 142, 146, 52, 48, 89, 54, 121, 212, 122, 60, 28, 42
};

static float dot3(const float a[], float x, float y, float z) {
    return a[0]*x + a[1]*y + a[2]*z;
}

static float lerp(float a, float b, float t) {
    return (1 - t) * a + t * b;
}

static float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

#define FASTFLOOR(x)  (((x) >= 0) ? (int)(x) : (int)(x)-1)

static float Perlin(float x, float y, float z) {
    /* Find grid points */
    int gx = FASTFLOOR(x);
    int gy = FASTFLOOR(y);
    int gz = FASTFLOOR(z);
    
    /* Relative coords within grid cell */
    float rx = x - gx;
    float ry = y - gy;
    float rz = z - gz;
    
    /* Wrap cell coords */
    gx = gx & 255;
    gy = gy & 255;
    gz = gz & 255;
    
    /* Calculate gradient indices */
    unsigned int gi[8];
    for (int i = 0; i < 8; i++)
        gi[i] = perm[gx+((i>>2)&1)+perm[gy+((i>>1)&1)+perm[gz+(i&1)]]] % 12;
    
    /* Noise contribution from each corner */
    float n[8];
    for (int i = 0; i < 8; i++)
        n[i] = dot3(grad3[gi[i]], rx - ((i>>2)&1), ry - ((i>>1)&1), rz - (i&1));
    
    /* Fade curves */
    float u = fade(rx);
    float v = fade(ry);
    float w = fade(rz);
    
    /* Interpolate */
    float nx[4];
    for (int i = 0; i < 4; i++)
        nx[i] = lerp(n[i], n[4+i], u);
    
    float nxy[2];
    for (int i = 0; i < 2; i++)
        nxy[i] = lerp(nx[i], nx[2+i], v);
    
    return lerp(nxy[0], nxy[1], w);
}

static float Remap(float value, float from1, float to1, float from2, float to2) {
    return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
}

static unsigned char* PerlinFBM(int w, int h, float z, float xoff, float yoff, float scale, float lacunarity, float gain, int octaves, bool fadeOut) {
    float min = FLT_MAX, max = FLT_MIN;
    float *grid = malloc(w * h * sizeof(float));
    for (int x = 0; x < w; ++x)
        for (int y = 0; y < h; ++y) {
            float freq = 2.f,
                  amp  = 1.f,
                  tot  = 0.f,
                  sum  = 0.f;
            for (int i = 0; i < octaves; ++i) {
                sum  += Perlin(((xoff + x) / scale) * freq, ((yoff + y) / scale) * freq, z) * amp;
                tot  += amp;
                freq *= lacunarity;
                amp  *= gain;
            }
            grid[y * w + x] = sum = (sum / tot);
            if (sum < min)
                min = sum;
            if (sum > max)
                max = sum;
        }
    
    unsigned char *result = malloc(w * h * sizeof(unsigned char));
    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++) {
            float height = 255.f - (255.f * Remap(grid[y * w + x], min, max, 0, 1.f));
            float grad = fadeOut ? sqrtf(powf(w / 2 - x, 2.f) + powf(h / 2 - y, 2.f)) : 0.f;
            float final = height - grad;
            result[y * w + x] = (unsigned char)final;
        }
    free(grid);
    return result;
}

typedef float Vec2 __attribute__((ext_vector_type(2)));

typedef struct {
    Vec2 position, texcoord;
} Vertex;

typedef sg_image Texture;

static Texture NewTexture(int w, int h) {
    return sg_make_image(&(sg_image_desc) {
        .width = w,
        .height = h,
        .usage = SG_USAGE_STREAM
    });
}

void DestroyTexture(Texture texture) {
    if (sg_query_image_state(texture) == SG_RESOURCESTATE_VALID)
        sg_destroy_image(texture);
}

typedef struct {
    int *buf;
    unsigned int w, h;
} Bitmap;

static Bitmap NewBitmap(unsigned int w, unsigned int h) {
    return (Bitmap) {
        .w = w,
        .h = h,
        .buf = malloc(w * h * sizeof(int))
    };
}

static void DestroyBitmap(Bitmap *bitmap) {
    if (bitmap && bitmap->buf)
        free(bitmap->buf);
}

#define DEFAULT_CANVAS_SIZE 512

typedef struct {
    int canvasWidth;
    int canvasHeight;
    float xoff;
    float yoff;
    float zoff;
    float scale;
    float lacunarity;
    float gain;
    int octaves;
    int fadeOut;
} Settings;

static Settings settings = {
    .canvasWidth = DEFAULT_CANVAS_SIZE,
    .canvasHeight = DEFAULT_CANVAS_SIZE,
    .xoff = 0.f,
    .yoff = 0.f,
    .zoff = 0.f,
    .scale = 200.f,
    .lacunarity = 2.f,
    .gain = .5f,
    .octaves = 8,
    .fadeOut = 0
};

static struct {
    Bitmap bitmap;
    sg_pass_action pass_action;
    sg_pipeline pipeline;
    sg_bindings binding;
    Texture texture;
    Vertex vertex[6];
    bool update;
    float zoom;
    float delta;
} state;

void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext()
    });
    
    snk_setup(&(snk_desc_t) {});
    
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .action=SG_ACTION_CLEAR, .value={.1f, .1f, .1f, 1.f} }
    };
    
    state.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .shader = sg_make_shader(default_program_shader_desc(sg_query_backend())),
        .layout = {
            .buffers[0].stride = sizeof(Vertex),
            .attrs = {
                [ATTR_default_vs_position].format=SG_VERTEXFORMAT_FLOAT2,
                [ATTR_default_vs_texcoord].format=SG_VERTEXFORMAT_FLOAT2
            }
        },
        .colors[0] = {
            .blend = {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .op_rgb = SG_BLENDOP_ADD,
                .src_factor_alpha = SG_BLENDFACTOR_ONE,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .op_alpha = SG_BLENDOP_ADD
            }
        }
    });
    
    state.texture = NewTexture(settings.canvasWidth, settings.canvasHeight);
    state.binding = (sg_bindings) {
        .fs_images[SLOT_tex] = state.texture,
        .vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc) {
            .usage = SG_USAGE_STREAM,
            .size = 6 * sizeof(Vertex)
        })
    };
    
    state.bitmap = NewBitmap(settings.canvasWidth, settings.canvasHeight);
    state.update = true;
    state.zoom = 1.f;
}

#define PHI 1.618033988749895f

#if !defined(MIN)
#define MIN(a, b) (a < b ? a : b)
#endif
#if !defined(MAX)
#define MAX(a, b) (a > b ? a : b)
#endif
#define CLAMP(n, min, max) (MIN(MAX(n, min), max))

#define ToRGB(H) (int)((255 << 24) | ((H) << 16) | ((H) << 8) | (H))

void frame(void) {
    state.delta = (float)(sapp_frame_duration() * 60.0);
    
    struct nk_context *ctx = snk_new_frame();
    Settings tmp;
    memcpy(&tmp, &settings, sizeof(Settings));
    
    if (nk_begin(ctx, "Settings", nk_rect(20, 20, 300, (int)(300.f * PHI)), NK_WINDOW_BORDER | NK_WINDOW_SCALABLE | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE)) {
        if (nk_tree_push(ctx, NK_TREE_TAB, "Size", NK_MAXIMIZED)) {
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
            nk_checkbox_label(ctx, "Apply circular gradient", &tmp.fadeOut);
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);
    
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
        tmp.octaves != settings.octaves ||
        tmp.fadeOut != settings.fadeOut)
        state.update = true;
    
    if (state.update) {
        memcpy(&settings, &tmp, sizeof(Settings));
        unsigned char *heightmap = PerlinFBM(settings.canvasWidth, settings.canvasHeight, settings.zoff, settings.xoff, settings.yoff, settings.scale, settings.lacunarity, settings.gain, settings.octaves, settings.fadeOut);
        for (int x = 0; x < settings.canvasWidth; x++)
            for (int y = 0; y < settings.canvasHeight; y++) {
                int i = y * settings.canvasWidth + x;
                unsigned char h = heightmap[i];
                state.bitmap.buf[i] = ToRGB(h);
            }
        sg_update_image(state.texture, &(sg_image_data) {
            .subimage[0][0] = {
                .ptr  = state.bitmap.buf,
                .size = state.bitmap.w * state.bitmap.h * sizeof(int)
            }
        });
        state.update = false;
    }
    
    Vec2 size = {settings.canvasWidth, settings.canvasHeight};
    Vec2 viewport = {sapp_width(),sapp_height()};
    Vec2 position = (viewport / 2.f) - (size / 2.f);
    
    Vec2 quad[4] = {
        {position.x, position.y + size.y}, // bottom left
        {position.x + size.x, position.y + size.y}, // bottom right
        {position.x + size.x, position.y }, // top right
        {position.x, position.y }, // top left
    };
    float vw =  2.f / viewport.x;
    float vh = -2.f / viewport.y;
    for (int j = 0; j < 4; j++)
        quad[j] = (Vec2) {
            vw * quad[j].x + -1.f,
            vh * quad[j].y +  1.f
        } * state.zoom;
    
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
        state.vertex[i] = (Vertex) {
            .position = quad[indices[i]],
            .texcoord = vtexquad[indices[i]]
        };
    
    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pipeline);
    sg_update_buffer(state.binding.vertex_buffers[0], &(sg_range) {
        .ptr = state.vertex,
        .size = 6 * sizeof(Vertex)
    });
    sg_apply_bindings(&state.binding);
    sg_draw(0, 6, 1);
    
    snk_render(sapp_width(), sapp_height());
    sg_end_pass();
    sg_commit();
}

void input(const sapp_event *e) {
    snk_handle_event(e);
    switch (e->type) {
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            state.zoom = CLAMP(state.zoom + (e->scroll_y * state.delta), .1f, 10.f);
            break;
        default:
            break;
    }
}

void cleanup(void) {
    DestroyBitmap(&state.bitmap);
    DestroyTexture(state.texture);
    sg_destroy_pipeline(state.pipeline);
    sg_destroy_buffer(state.binding.vertex_buffers[0]);
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = input,
        .cleanup_cb = cleanup,
        .width = 1280,
        .height = 720,
        .sample_count = 4,
        .gl_force_gles2 = true,
        .window_title = "perlin",
        .icon.sokol_default = true,
    };
}
