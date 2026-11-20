#pragma once
#include <cmath>

struct Vector3 { float x, y, z; };
struct Vector2 { float x, y; };
struct Matrix4x4 { float m[4][4]; };

inline Vector3 CalcAngle(Vector3 src, Vector3 dst) {
    Vector3 angles;
    Vector3 delta = { dst.x - src.x, dst.y - src.y, dst.z - src.z };
    float hyp = sqrt(delta.x * delta.x + delta.y * delta.y);
    angles.x = atan2(-delta.z, hyp) * (180.0f / 3.14159265f);
    angles.y = atan2(delta.y, delta.x) * (180.0f / 3.14159265f);
    angles.z = 0.0f;
    return angles;
}

inline bool WorldToScreen(Vector3 pos, Vector2& screen, Matrix4x4 matrix, int width, int height) {
    float w = matrix.m[3][0] * pos.x + matrix.m[3][1] * pos.y + matrix.m[3][2] * pos.z + matrix.m[3][3];
    if (w < 0.01f) return false;
    
    float x = matrix.m[0][0] * pos.x + matrix.m[0][1] * pos.y + matrix.m[0][2] * pos.z + matrix.m[0][3];
    float y = matrix.m[1][0] * pos.x + matrix.m[1][1] * pos.y + matrix.m[1][2] * pos.z + matrix.m[1][3];
    
    screen.x = (width / 2.0f) + (width / 2.0f) * x / w;
    screen.y = (height / 2.0f) - (height / 2.0f) * y / w;
    return true;
}