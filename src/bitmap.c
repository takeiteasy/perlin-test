//
//  bitmap.c
//  sokol
//
//  Created by George Watson on 23/02/2023.
//

#include "bitmap.h"

Texture NewTexture(int w, int h) {
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

Bitmap NewBitmap(unsigned int w, unsigned int h) {
    return (Bitmap) {
        .w = w,
        .h = h,
        .buf = malloc(w * h * sizeof(int))
    };
}

#if !WEB_BUILD
#define SVPNG_LINKAGE static
#include "svpng.inc"

void ExportBitmap(Bitmap *bitmap, const char *path) {
    FILE *fp = fopen(path, "wb");
    unsigned char *out = malloc(bitmap->w * bitmap->h * 3 * sizeof(unsigned char));
    unsigned char *p = out;
    for (int x = 0; x < bitmap->w; x++)
        for (int y = 0; y < bitmap->h; y++) {
            int c = bitmap->buf[y * bitmap->w + x];
            *p++ = (unsigned char)( c        & 0xFF);
            *p++ = (unsigned char)((c >> 8)  & 0xFF);
            *p++ = (unsigned char)((c >> 16) & 0xFF);
        }
    svpng(fp, bitmap->w, bitmap->h, out, 0);
    free(out);
    fclose(fp);
}
#else
#endif

void DestroyBitmap(Bitmap *bitmap) {
    if (bitmap && bitmap->buf)
        free(bitmap->buf);
}
