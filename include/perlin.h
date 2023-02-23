//
//  perlin.h
//  sokol
//
//  Created by George Watson on 23/02/2023.
//

#ifndef perlin_h
#define perlin_h
#include <float.h>
#include <stdlib.h>

float Perlin(float x, float y, float z);
unsigned char* PerlinFBM(int w, int h, float z, float xoff, float yoff, float scale, float lacunarity, float gain, int octaves);

#endif /* perlin_h */
