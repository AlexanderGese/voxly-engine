#include "debugdraw_shapes.h"
#include "../../math/mat4.h"
#include <math.h>
#define DD_TAU 6.28318530718f
static void basis_from_normal(vec3 n, vec3 *t, vec3 *b) {
    n = vec3_normalize(n);
    float s = n.z >= 0.0f ? 1.0f : -1.0f;
    float a = -1.0f / (s + n.z);
    float bx = n.x * n.y * a;
    *t = vec3_new(1.0f + s * n.x * n.x * a, s * bx, -s * n.x);
    *b = vec3_new(bx, s + n.y * n.y * a, -n.y);
}

void debugdraw_circle(debugdraw *dd, vec3 center, vec3 normal,
                      float radius, int segments, ddcolor c) {
    if (segments < 3) segments = 24;
vec3 t, b;
basis_from_normal(normal, &t, &b);
vec3 prev = VEC3_ZERO;
for (int i = 0;
i <= segments;
i++) {
        float ang = DD_TAU * (float)i / (float)segments;
        float cs = cosf(ang) * radius;
        float sn = sinf(ang) * radius;
        vec3 p = vec3_add(center,
                          vec3_add(vec3_scale(t, cs), vec3_scale(b, sn)));
        if (i > 0) debugdraw_emit_line(dd, prev, p, c, c);
        prev = p;
    }
}

void debugdraw_sphere(debugdraw *dd, vec3 center, float radius,
                      int segments, ddcolor c) {
    if (segments < 3) segments = 16;
debugdraw_circle(dd, center, VEC3_RIGHT, radius, segments, c);
debugdraw_circle(dd, center, VEC3_UP,    radius, segments, c);
debugdraw_circle(dd, center, VEC3_FWD,   radius, segments, c);
}

void debugdraw_cross(debugdraw *dd, vec3 center, float size, ddcolor c) {
    float h = size * 0.5f;
    debugdraw_emit_line(dd, vec3_new(center.x - h, center.y, center.z),
                            vec3_new(center.x + h, center.y, center.z), c, c);
    debugdraw_emit_line(dd, vec3_new(center.x, center.y - h, center.z),
                            vec3_new(center.x, center.y + h, center.z), c, c);
    debugdraw_emit_line(dd, vec3_new(center.x, center.y, center.z - h),
                            vec3_new(center.x, center.y, center.z + h), c, c);
}

void debugdraw_arrow(debugdraw *dd, vec3 from, vec3 to,
                     float head_size, ddcolor c) {
    debugdraw_emit_line(dd, from, to, c, c);
vec3 dir = vec3_sub(to, from);
float len = vec3_length(dir);
if (len < 1e-5f) return;
dir = vec3_scale(dir, 1.0f / len);
if (head_size <= 0.0f) head_size = len * 0.2f;
if (head_size > len)   head_size = len * 0.5f;
vec3 base = vec3_sub(to, vec3_scale(dir, head_size));
vec3 t, b;
basis_from_normal(dir, &t, &b);
float r = head_size * 0.4f;
for (int i = 0;
i < 4;
int idx = 0;
for (int zi = -1;
zi <= 1;
zi += 2)
    for (int yi = -1;
yi <= 1;
yi += 2)
    for (int xi = -1;
xi <= 1;
xi += 2)
        corners[idx++] = mat4_mul_vec3(model,
            vec3_new(xi * 0.5f, yi * 0.5f, zi * 0.5f));
;
for (int i = 0;
i < 12;
i++)
        debugdraw_emit_line(dd, corners[e[i][0]], corners[e[i][1]], c, c);
}
