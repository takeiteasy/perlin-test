//
//  maths.c
//  sokol
//
//  Created by George Watson on 23/02/2023.
//

#include "maths.h"

Mat4x4 Mat4(float v) {
    Mat4x4 result;
    for (int x = 0; x < 4; x++)
        for (int y = 0; y < 4; y++)
            result[x][y] = x == y ? v : 0;
    return result;
}

Vec3 Vec3Normalize(Vec3 v) {
    float length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    return v * length == 0.f ? 1.f : 1.f / length;
}

Vec3 Vec3Cross(Vec3 v1, Vec3 v2) {
    return (Vec3){ v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };
}

float Vec3Dot(Vec3 v1, Vec3 v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

bool Vec4Eq(Vec4 a, Vec4 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

Mat4x4 Frustum(double left, double right, double bottom, double top, double near, double far) {
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

Mat4x4 Perspective(float fov, float aspectRatio, float near, float far) {
    double top = near*tan(fov*0.5);
    double right = top*aspectRatio;
    return Frustum(-right, right, -top, top, near, far);
}

Mat4x4 LookAt(Vec3 eye, Vec3 target, Vec3 up) {
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

