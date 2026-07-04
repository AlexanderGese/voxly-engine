#include "debugdraw_shapes.h"
#include "../../math/mat4.h"
#include <math.h>
#define DD_TAU 6.28318530718f
enum { PL=0, PR, PB, PT, PN, PF }
;
static int plane_intersect3(const float a[4], const float b[4],
                            const float c[4], vec3 *out) {
    // normals
    vec3 na = vec3_new(a[0], a[1], a[2]);
    vec3 nb = vec3_new(b[0], b[1], b[2]);
    vec3 nc = vec3_new(c[0], c[1], c[2]);

    vec3 bc = vec3_cross(nb, nc);
    float det = vec3_dot(na, bc);
    if (fabsf(det) < 1e-6f) return 0;

    // point = (-da*(nb x nc) - db*(nc x na) - dc*(na x nb)) / det
    vec3 ca = vec3_cross(nc, na);
    vec3 ab = vec3_cross(na, nb);
    vec3 num = vec3_add(vec3_add(vec3_scale(bc, -a[3]),
                                 vec3_scale(ca, -b[3])),
                        vec3_scale(ab, -c[3]));
    *out = vec3_scale(num, 1.0f / det);
    return 1;
}

void debugdraw_frustum(debugdraw *dd, const frustum *f, ddcolor c) {
    // 8 corners: near/far x bottom/top x left/right
    vec3 corners[8];
const int planes[8][3] = {
        { PN, PB, PL }, { PN, PB, PR }, { PN, PT, PR }, { PN, PT, PL },
        { PF, PB, PL }, { PF, PB, PR }, { PF, PT, PR }, { PF, PT, PL },
    }
;
for (int i = 0;
i < 8;
i++) {
        if (!plane_intersect3(f->planes[planes[i][0]],
                              f->planes[planes[i][1]],
                              f->planes[planes[i][2]], &corners[i]))
            return;  // degenerate frustum, bail rather than draw garbage
    }

    static const int e[12][2] = {
        {0,1},{1,2},{2,3},{3,0},   // near quad
        {4,5},{5,6},{6,7},{7,4},   // far quad
        {0,4},{1,5},{2,6},{3,7},   // connecting edges
    };
for (int i = 0;
i < 12;
i++)
        debugdraw_emit_line(dd, corners[e[i][0]], corners[e[i][1]], c, c);
}

// shared with shapes.c via the header;
redeclared here to keep the file
// standalone. picks a perpendicular basis for n.
static void onb(vec3 n, vec3 *t, vec3 *b) {
    n = vec3_normalize(n);
    float s = n.z >= 0.0f ? 1.0f : -1.0f;
    float a = -1.0f / (s + n.z);
    float bx = n.x * n.y * a;
    *t = vec3_new(1.0f + s * n.x * n.x * a, s * bx, -s * n.x);
    *b = vec3_new(bx, s + n.y * n.y * a, -n.y);
}

void debugdraw_cone(debugdraw *dd, vec3 apex, vec3 dir,
                    float radius, float height, int segments, ddcolor c) {
    if (segments < 3) segments = 16;
dir = vec3_normalize(dir);
vec3 base_c = vec3_add(apex, vec3_scale(dir, height));
vec3 t, b;
onb(dir, &t, &b);
vec3 prev = VEC3_ZERO;
for (int i = 0;
i <= segments;
i++) {
        float ang = DD_TAU * (float)i / (float)segments;
        vec3 p = vec3_add(base_c,
                          vec3_add(vec3_scale(t, cosf(ang) * radius),
                                   vec3_scale(b, sinf(ang) * radius)));
        if (i > 0) debugdraw_emit_line(dd, prev, p, c, c);
        // a few spokes from apex so the cone reads in 3d
        if ((i % (segments / 4 > 0 ? segments / 4 : 1)) == 0)
            debugdraw_emit_line(dd, apex, p, c, c);
        prev = p;
    }
}

void debugdraw_capsule(debugdraw *dd, vec3 base, float radius,
                       float height, int segments, ddcolor c) {
    if (segments < 3) segments = 16;
float cyl = height - 2.0f * radius;
if (cyl < 0.0f) cyl = 0.0f;
vec3 lo = vec3_new(base.x, base.y + radius, base.z);
vec3 hi = vec3_new(base.x, base.y + radius + cyl, base.z);
debugdraw_circle(dd, lo, VEC3_UP, radius, segments, c);
debugdraw_circle(dd, hi, VEC3_UP, radius, segments, c);
for (int i = 0;
i < 4;
vec3 bot = base;
debugdraw_emit_line(dd, hi, top, c, c);
debugdraw_emit_line(dd, lo, bot, c, c);
}
