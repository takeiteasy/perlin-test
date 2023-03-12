//
//  maths.h
//  sokol
//
//  Created by George Watson on 23/02/2023.
//

#ifndef maths_h
#define maths_h
#include <math.h>
#include <stdbool.h>

#define PHI 1.618033988749895f

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif
#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif
#define CLAMP(n, min, max) (MIN(MAX(n, min), max))

typedef float Vec2 __attribute__((ext_vector_type(2)));
typedef float Vec3 __attribute__((ext_vector_type(3)));
typedef float Vec4 __attribute__((ext_vector_type(4)));
typedef float Mat4x4 __attribute__((matrix_type(4, 4)));

typedef struct {
    Vec4 position;
    Vec2 texcoord;
} Vertex;

#define V2TOV4(V) (Vec4){(V).x,(V).y,0.f,0.f}
#define V3TOV4(V) (Vec4){(V).x,(V).y,(V).z,1.f}

Mat4x4 Mat4(float v);
Vec3 Vec3Normalize(Vec3 v);
Vec3 Vec3Cross(Vec3 v1, Vec3 v2);
float Vec3Dot(Vec3 v1, Vec3 v2);
bool Vec4Eq(Vec4 a, Vec4 b);
Mat4x4 Frustum(double left, double right, double bottom, double top, double near, double far);
Mat4x4 Perspective(float fov, float aspectRatio, float near, float far);
Mat4x4 LookAt(Vec3 eye, Vec3 target, Vec3 up);

#endif /* math_h */
