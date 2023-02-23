#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_IMPLEMENTATION
#include "nuklear.h"
#define LUA_IMPL
#include "minilua.h"
#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_nuklear.h"
#include "default.glsl.h"

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#define WEB_BUILD 1
#else
#define WEB_BUILD 0
#endif

#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define PLATFORM_MAC
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define PLATFORM_WINDOWS
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define PLATFORM_LINUX
#endif

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

static unsigned char* PerlinFBM(int w, int h, float z, float xoff, float yoff, float scale, float lacunarity, float gain, int octaves) {
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
            result[y * w + x] = (unsigned char)height;
        }
    free(grid);
    return result;
}

typedef float Vec2 __attribute__((ext_vector_type(2)));
typedef float Vec3 __attribute__((ext_vector_type(3)));
typedef float Vec4 __attribute__((ext_vector_type(4)));
typedef float Mat4x4 __attribute__((matrix_type(4, 4)));

#define V2TOV4(V) (Vec4){(V).x,(V).y,0.f,0.f}

static Mat4x4 Mat4(float v) {
    Mat4x4 result;
    for (int i = 0; i < 4; i++)
        result[i][i] = v;
    return result;
}

static Mat4x4 Frustum(double left, double right, double bottom, double top, double near, double far) {
    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(far - near);

    Mat4x4 result = Mat4(0.f);
    result[0][0] = ((float)near*2.0f)/rl;
    result[1][1] = ((float)near*2.0f)/tb;
    result[0][2] = ((float)right + (float)left)/rl;
    result[1][2] = ((float)top + (float)bottom)/tb;
    result[2][2] = -((float)far + (float)near)/fn;
    result[3][2] = -1.0f;
    result[2][3] = -((float)far*(float)near*2.0f)/fn;
    return result;
}

static Mat4x4 Perspective(float fov, float aspectRatio, float near, float far) {
    double top = near*tan(fov*0.5);
    double right = top*aspectRatio;
    return Frustum(-right, right, -top, top, near, far);
}

static Vec3 Vec3Normalize(Vec3 v) {
    float length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    return v * length == 0.f ? 1.f : 1.f / length;
}

static Vec3 Vec3Cross(Vec3 v1, Vec3 v2) {
    return (Vec3){ v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };
}

static float Vec3Dot(Vec3 v1, Vec3 v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

static Mat4x4 LookAt(Vec3 eye, Vec3 target, Vec3 up) {
    Vec3 vz = Vec3Normalize(eye - target);
    Vec3 vx = Vec3Normalize(Vec3Cross(up, vz));
    Vec3 vy = Vec3Cross(vz, vx);
    
    Mat4x4 result = Mat4(0.f);
    result[0][0] = vx.x;
    result[1][0] = vy.x;
    result[2][0] = vz.x;
    result[3][0] = 0.0f;
    result[0][1] = vx.y;
    result[1][1] = vy.y;
    result[2][1] = vz.y;
    result[3][1] = 0.0f;
    result[0][2] = vx.z;
    result[1][2] = vy.z;
    result[2][2] = vz.z;
    result[3][2] = 0.0f;
    result[0][3] = -Vec3Dot(vx, eye);
    result[1][3] = -Vec3Dot(vy, eye);
    result[2][3] = -Vec3Dot(vz, eye);
    result[3][3] = 1.0f;
    return result;
}

typedef struct {
    Vec4 position;
    Vec2 texcoord;
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

#define vector__sbraw(a) ((int *)(void *)(a)-2)
#define vector__sbm(a) vector__sbraw(a)[0]
#define vector__sbn(a) vector__sbraw(a)[1]

#define vector__sbneedgrow(a, n) ((a) == 0 || vector__sbn(a) + (n) >= vector__sbm(a))
#define vector__sbmaybegrow(a, n) (vector__sbneedgrow(a, (n)) ? vector__sbgrow(a, n) : 0)
#define vector__sbgrow(a, n) (*((void **)&(a)) = vector__sbgrowf((a), (n), sizeof(*(a))))

#define DestroyVector(a) ((a) ? free(vector__sbraw(a)), 0 : 0)
#define VectorAppend(a, v) (vector__sbmaybegrow(a, 1), (a)[vector__sbn(a)++] = (v))
#define VectorCount(a) ((a) ? vector__sbn(a) : 0)

static void *vector__sbgrowf(void *arr, int increment, int itemsize) {
    int dbl_cur = arr ? 2 * vector__sbm(arr) : 0;
    int min_needed = VectorCount(arr) + increment;
    int m = dbl_cur > min_needed ? dbl_cur : min_needed;
    int *p = realloc(arr ? vector__sbraw(arr) : 0, itemsize * m + sizeof(int) * 2);
    if (p) {
        if (!arr)
            p[1] = 0;
        p[0] = m;
        return p + 2;
    } else {
#ifdef VECTOR_OUT_OF_MEMORY
        VECTOR_OUT_OF_MEMORY;
#endif
        return (void *)(2 * sizeof(int)); // try to force a NULL pointer exception later
    }
}

#if defined(PLATFORM_WINDOWS)
#include <io.h>
#define F_OK 0
#define access _access
#define PATH_SEPERATOR "\\"
#else
#include <dirent.h>
#include <unistd.h>
#define PATH_SEPERATOR "/"
#endif

static const char* FileExt(const char *path) {
    const char *dot = strrchr(path, '.');
    return !dot || dot == path ? NULL : dot + 1;
}

static const char** FindFiles(const char *ext) {
#if WEB_BUILD
    return NULL;
#else
#if defined(PLATFORM_WINDOWS)
    //! TODO: FindFiles() Windows
    return NULL;
#else
    const char **result = NULL;
    unsigned long extLength = strlen(ext);
    static const char *path = "assets";
    DIR *dir = opendir(path);
    struct dirent *d;
    while ((d = readdir(dir))) {
        if (d->d_type == DT_REG) {
            const char *newExt = FileExt(d->d_name);
            if (newExt && !strncmp(newExt, ext, extLength))
                VectorAppend(result, strdup(d->d_name));
        }
    }
    closedir(dir);
    return result;
#endif
#endif
}

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
    const char **models;
    int currentModel;
    const char **scripts;
    int currentScript;
    lua_State *luaState;
} state;

typedef struct {
    Bitmap *bitmap;
} LuaBitmap;

static int LuaBitmapPSet(lua_State *L) {
    LuaBitmap *lbitmap = (LuaBitmap*)luaL_checkudata(L, 1, "Bitmap");
    unsigned int x = (unsigned int)luaL_checkinteger(L, 2);
    unsigned int y = (unsigned int)luaL_checkinteger(L, 3);
    int color = (int)luaL_checkinteger(L, 4);
    lbitmap->bitmap->buf[y * lbitmap->bitmap->w + x] = color;
    return 0;
}

static int LuaBitmapPGet(lua_State *L) {
    LuaBitmap *lbitmap = (LuaBitmap*)luaL_checkudata(L, 1, "Bitmap");
    unsigned int x = (unsigned int)luaL_checkinteger(L, 2);
    unsigned int y = (unsigned int)luaL_checkinteger(L, 3);
    int color = lbitmap->bitmap->buf[y * lbitmap->bitmap->w + x];
    lua_pushinteger(L, color);
    return 1;
}

static int LuaBitmapGet(lua_State *L) {
    LuaBitmap *lbitmap = (LuaBitmap*)luaL_checkudata(L, 1, "Bitmap");
    unsigned int x = (unsigned int)luaL_checkinteger(L, 2);
    unsigned int y = (unsigned int)luaL_checkinteger(L, 3);
    unsigned char color = lbitmap->bitmap->buf[y * lbitmap->bitmap->w + x] & 0xFF;
    lua_pushinteger(L, color);
    return 1;
}

static int LuaBitmapWidth(lua_State *L) {
    LuaBitmap *lbitmap = (LuaBitmap*)luaL_checkudata(L, 1, "Bitmap");
    lua_pushinteger(L, lbitmap->bitmap->w);
    return 1;
}

static int LuaBitmapHeight(lua_State *L) {
    LuaBitmap *lbitmap = (LuaBitmap*)luaL_checkudata(L, 1, "Bitmap");
    lua_pushinteger(L, lbitmap->bitmap->h);
    return 1;
}

static const struct luaL_Reg BitmapMethods[] = {
    {"pset", LuaBitmapPSet},
    {"pget", LuaBitmapPGet},
    {"get", LuaBitmapGet},
    {"width", LuaBitmapWidth},
    {"height", LuaBitmapHeight},
    {NULL, NULL}
};

static const struct luaL_Reg BitmapFunctions[] = {
    {NULL, NULL}
};

static void LuaFail(lua_State *L, char *msg, bool die) {
    fprintf(stderr, "\nERROR:\n  %s: %s\n\n", msg, lua_tostring(L, -1));
    if (die)
        exit(1);
}

#define RGB(R, G, B) (int)((255 << 24) | ((unsigned char)(B) << 16) | ((unsigned char)(G) << 8) | (unsigned char)(R))

static int LuaRGB(lua_State *L) {
    int r = (int)luaL_checkinteger(L, 1);
    int g = (int)luaL_checkinteger(L, 2);
    int b = (int)luaL_checkinteger(L, 3);
    int c = RGB(r, g, b);
    lua_pushinteger(L, c);
    return 1;
}

static int LuaSettings(lua_State *L) {
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

static int LuaDelta(lua_State *L) {
    lua_pushnumber(L, (lua_Number)state.delta);
    return 1;
}

static void LoadLuaScript(void) {
    if (state.luaState)
        lua_close(state.luaState);
    
    state.luaState = luaL_newstate();
    luaL_openlibs(state.luaState);
    
    luaL_newmetatable(state.luaState, "Bitmap");
    lua_pushvalue(state.luaState, -1);
    lua_setfield(state.luaState, -2, "__index");
    luaL_setfuncs(state.luaState, BitmapMethods, 0);
    luaL_newlib(state.luaState, BitmapFunctions);
    
    lua_pushcfunction(state.luaState, LuaRGB);
    lua_setglobal(state.luaState, "RGB");
    lua_pushcfunction(state.luaState, LuaSettings);
    lua_setglobal(state.luaState, "Setting");
    lua_pushcfunction(state.luaState, LuaDelta);
    lua_setglobal(state.luaState, "Delta");
    
    char asset[1024];
    sprintf(asset, "assets%s%s", PATH_SEPERATOR, state.scripts[state.currentScript-1]);
    if (luaL_dofile(state.luaState, asset))
        LuaFail(state.luaState, "Errors found in lua script", false);
}

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
    
    state.models = FindFiles("obj");
    state.currentModel = 0;
    state.scripts = FindFiles("lua");
    state.currentScript = 0;
    state.luaState = NULL;
    
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

#if !WEB_BUILD
#include "svpng.inc"

static void ExportPNG(void) {
    char path[256];
    time_t raw = time(NULL);
    struct tm *t = localtime(&raw);
    strftime(path, 256, "Perlin %G-%m-%d at %H.%M.%S.png", t);
    FILE *fp = fopen(path, "wb");
    
    unsigned char *out = malloc(state.bitmap.w * state.bitmap.h * 3 * sizeof(unsigned char));
    unsigned char *p = out;
    for (int x = 0; x < state.bitmap.w; x++)
        for (int y = 0; y < state.bitmap.h; y++) {
            int c = state.bitmap.buf[y * state.bitmap.w + x];
            *p++ = (unsigned char)( c        & 0xFF);
            *p++ = (unsigned char)((c >> 8)  & 0xFF);
            *p++ = (unsigned char)((c >> 16) & 0xFF);
        }
    svpng(fp, state.bitmap.w, state.bitmap.h, out, 0);
    free(out);
    fclose(fp);
}
#endif

#define PHI 1.618033988749895f

#if !defined(MIN)
#define MIN(a, b) (a < b ? a : b)
#endif
#if !defined(MAX)
#define MAX(a, b) (a > b ? a : b)
#endif
#define CLAMP(n, min, max) (MIN(MAX(n, min), max))

void frame(void) {
    state.delta = (float)(sapp_frame_duration() * 60.0);
    
    if (state.currentScript != 0) {
        lua_getglobal(state.luaState, "preframe");
        if (lua_isfunction(state.luaState, -1)) {
            if (lua_pcall(state.luaState, 0, 0, 0))
                LuaFail(state.luaState, "Failed to execute Lua script", false);
        }
    }
    
    struct nk_context *ctx = snk_new_frame();
    Settings tmp;
    memcpy(&tmp, &settings, sizeof(Settings));
    
    int currentModel = state.currentModel, currentScript = state.currentScript;
    bool resetValues = false;
    if (nk_begin(ctx, "Settings", nk_rect(0, 0, 300, (int)(300.f * PHI)), NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE)) {
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
        if (nk_button_label(ctx, "Export"))
            ExportPNG();
#endif
    }
    nk_end(ctx);
   
    if (!nk_window_is_any_hovered(ctx))
        state.zoom = CLAMP(state.zoom + (state.scrollY * state.delta), .1f, 10.f);
    
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
        } else
            LoadLuaScript();
    }
    
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
        
        if (state.currentScript != 0) {
            lua_getglobal(state.luaState, "frame");
            LuaBitmap *lbitmap = (LuaBitmap*)lua_newuserdata(state.luaState, sizeof(LuaBitmap));
            lbitmap->bitmap = &state.bitmap;
            luaL_getmetatable(state.luaState, "Bitmap");
            lua_setmetatable(state.luaState, -2);
            if (lua_pcall(state.luaState, 1, 0, 0))
                LuaFail(state.luaState, "Failed to execute Lua script", false);
        }
        
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
    for (int i = 0; i < VectorCount(state.models); i++)
        free((void*)state.models[i]);
    DestroyVector(state.models);
    for (int i = 0; i < VectorCount(state.scripts); i++)
        free((void*)state.scripts[i]);
    DestroyVector(state.scripts);
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
