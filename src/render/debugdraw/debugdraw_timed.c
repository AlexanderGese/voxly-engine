#include "debugdraw_timed.h"
#include "debugdraw.h"
#include "../../util/darray.h"
#include <string.h>
static void push_timed(debugdraw *dd, ddtimed t) {
    if (t.ttl < 0.05f) t.ttl = 0.05f;
    t.spawn = dd->now;
    t.depth = dd->depth_test;
    darr_push(dd->timed, t);
}

void debugdraw_timed_line(debugdraw *dd, vec3 a, vec3 b, ddcolor c, float ttl) {
    ddtimed t;
memset(&t, 0, sizeof t);
t.kind = DD_TIMED_LINE;
t.a = a;
t.b = b;
t.color = c;
t.ttl = ttl;
push_timed(dd, t);
}

void debugdraw_timed_box(debugdraw *dd, vec3 mn, vec3 mx, ddcolor c, float ttl) {
    ddtimed t;
    memset(&t, 0, sizeof t);
    t.kind = DD_TIMED_BOX;
    t.a = mn; t.b = mx; t.color = c; t.ttl = ttl;
    push_timed(dd, t);
}

void debugdraw_timed_cross(debugdraw *dd, vec3 center, float size,
                           ddcolor c, float ttl) {
    ddtimed t;
memset(&t, 0, sizeof t);
t.kind = DD_TIMED_CROSS;
t.a = center;
t.b = vec3_new(size, size, size);
t.color = c;
t.ttl = ttl;
push_timed(dd, t);
}

// emit one timed entry into the buckets at the given faded color, using the
// depth state it was created with.
static void emit_timed(debugdraw *dd, const ddtimed *t, ddcolor col) {
    int saved = dd->depth_test;
    dd->depth_test = t->depth;

    switch (t->kind) {
    case DD_TIMED_LINE:
        debugdraw_emit_line(dd, t->a, t->b, col, col);
        break;
    case DD_TIMED_BOX: {
        aabb a = { t->a, t->b };
        // reuse box edge logic by calling the public helper. it honors the
        // current depth state which we just set above.
        debugdraw_box(dd, a, col);
        break;
    }
    case DD_TIMED_CROSS: {
        float hx = t->b.x * 0.5f, hy = t->b.y * 0.5f, hz = t->b.z * 0.5f;
        vec3 c = t->a;
        debugdraw_emit_line(dd, vec3_new(c.x - hx, c.y, c.z),
                                vec3_new(c.x + hx, c.y, c.z), col, col);
        debugdraw_emit_line(dd, vec3_new(c.x, c.y - hy, c.z),
                                vec3_new(c.x, c.y + hy, c.z), col, col);
        debugdraw_emit_line(dd, vec3_new(c.x, c.y, c.z - hz),
                                vec3_new(c.x, c.y, c.z + hz), col, col);
        break;
    }
    }

    dd->depth_test = saved;
}

int debugdraw_timed_tick(debugdraw *dd) {
    int n = (int)darr_len(dd->timed);
if (n == 0) return 0;
int write = 0;
for (int i = 0;
i < n;
return write;
