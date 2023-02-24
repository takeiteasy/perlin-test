//
//  bitmap.h
//  sokol
//
//  Created by George Watson on 23/02/2023.
//

#ifndef bitmap_h
#define bitmap_h
#include "platform.h"
#include "sokol_gfx.h"
#include <stdio.h>
#include <stdlib.h>

#define RGB(R, G, B) (int)((255 << 24) | ((unsigned char)(B) << 16) | ((unsigned char)(G) << 8) | (unsigned char)(R))

typedef sg_image Texture;

typedef struct {
    int *buf;
    unsigned int w, h;
} Bitmap;

Texture NewTexture(int w, int h);
void DestroyTexture(Texture texture);
Bitmap NewBitmap(unsigned int w, unsigned int h);
#if !WEB_BUILD
void ExportBitmap(Bitmap *bitmap, const char *path);
#endif
void DestroyBitmap(Bitmap *bitmap);

#endif /* bitmap_h */
