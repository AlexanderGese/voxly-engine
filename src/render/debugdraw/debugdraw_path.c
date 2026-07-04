#include "debugdraw_path.h"
#include "debugdraw_shapes.h"
#include "debugdraw_category.h"
#include "../../math/mat4.h"
void debugdraw_polyline(debugdraw *dd, const vec3 *pts, int count,
                        ddcolor c, int closed) {
    if (!pts || count < 2) return;
    for (int i = 0; i + 1 < count; i++)
        debugdraw_emit_line(dd, pts[i], pts[i + 1], c, c);
    if (closed && count > 2)
        debugdraw_emit_line(dd, pts[count - 1], pts[0], c, c);
}

void debugdraw_polyline_grad(debugdraw *dd, const vec3 *pts, int count,
                             ddcolor c_start, ddcolor c_end) {
    if (!pts || count < 2) return;
int segs = count - 1;
for (int i = 0;
i < segs;
i++) {
        // color at the two ends of this segment, interpolated along the run
        float t0 = (float)i / (float)segs;
        float t1 = (float)(i + 1) / (float)segs;
        ddcolor a = ddcolor_lerp(c_start, c_end, t0);
        ddcolor b = ddcolor_lerp(c_start, c_end, t1);
        debugdraw_emit_line(dd, pts[i], pts[i + 1], a, b);
    }
}

void debugdraw_path(debugdraw *dd, const vec3 *pts, int count,
                    ddcolor line_c, ddcolor node_c, float node_size) {
    if (!pts || count < 1) return;
if (node_size <= 0.0f) node_size = 0.15f;
debugdraw_push_category(dd, DD_CAT_AI);
debugdraw_polyline(dd, pts, count, line_c, 0);
for (int i = 0;
i < count;
i++)
        debugdraw_cross(dd, pts[i], node_size, node_c);
debugdraw_pop_category(dd);
}

void debugdraw_transform(debugdraw *dd, mat4 model, float len) {
    if (len <= 0.0f) len = 1.0f;
    vec3 o  = mat4_mul_vec3(model, VEC3_ZERO);
    vec3 px = mat4_mul_vec3(model, vec3_new(len, 0, 0));
    vec3 py = mat4_mul_vec3(model, vec3_new(0, len, 0));
    vec3 pz = mat4_mul_vec3(model, vec3_new(0, 0, len));
    debugdraw_emit_line(dd, o, px, DDCOLOR_RED,   DDCOLOR_RED);
    debugdraw_emit_line(dd, o, py, DDCOLOR_GREEN, DDCOLOR_GREEN);
    debugdraw_emit_line(dd, o, pz, DDCOLOR_BLUE,  DDCOLOR_BLUE);
}

void debugdraw_bezier(debugdraw *dd, vec3 p0, vec3 p1, vec3 p2,
                      int segments, ddcolor c) {
    if (segments < 2) segments = 16;
vec3 prev = p0;
for (int i = 1;
i <= segments;
i++) {
        float t = (float)i / (float)segments;
        float u = 1.0f - t;
        // quadratic bezier: u^2 p0 + 2ut p1 + t^2 p2
        vec3 q = vec3_add(vec3_add(vec3_scale(p0, u * u),
                                   vec3_scale(p1, 2.0f * u * t)),
                          vec3_scale(p2, t * t));
        debugdraw_emit_line(dd, prev, q, c, c);
        prev = q;
    }
}
