#include "treegen_turtle.h"
#include <math.h>

#define DEG2RAD (3.14159265358979323846f / 180.0f)

void treegen_turtle_init(treegen_turtle_state *t, float length, float radius) {
    treegen_turtle *c = &t->cur;
    c->pos    = VEC3_ZERO;
    c->head   = VEC3_UP;      // grow upward to start
    c->left   = VEC3_RIGHT;
    c->up     = VEC3_FWD;     // turtle's own up; world fwd works as the third axis
    c->length = length;
    c->radius = radius;
    t->sp = 0;
}

int treegen_turtle_push(treegen_turtle_state *t) {
    if (t->sp >= TREEGEN_STACK_MAX) return 0;
    t->stack[t->sp++] = t->cur;
    return 1;
}

int treegen_turtle_pop(treegen_turtle_state *t) {
    if (t->sp <= 0) return 0;
    t->cur = t->stack[--t->sp];
    return 1;
}

// rotate v about unit axis k by angle (rad). rodrigues' rotation formula.
static vec3 rot_axis(vec3 v, vec3 k, float ang) {
    float c = cosf(ang), s = sinf(ang);
    vec3 term1 = vec3_scale(v, c);
    vec3 term2 = vec3_scale(vec3_cross(k, v), s);
    vec3 term3 = vec3_scale(k, vec3_dot(k, v) * (1.0f - c));
    return vec3_add(vec3_add(term1, term2), term3);
}

// drift can creep in over many rotations; re-derive an orthonormal frame from
// head + up so we don't slowly turn into a parallelogram.
static void reortho(treegen_turtle *c) {
    c->head = vec3_normalize(c->head);
    c->left = vec3_normalize(vec3_cross(c->up, c->head));
    c->up   = vec3_cross(c->head, c->left);
}

void treegen_turtle_yaw(treegen_turtle_state *t, float deg) {
    treegen_turtle *c = &t->cur;
    float a = deg * DEG2RAD;
    c->head = rot_axis(c->head, c->up, a);
    c->left = rot_axis(c->left, c->up, a);
    reortho(c);
}

void treegen_turtle_pitch(treegen_turtle_state *t, float deg) {
    treegen_turtle *c = &t->cur;
    float a = deg * DEG2RAD;
    c->head = rot_axis(c->head, c->left, a);
    c->up   = rot_axis(c->up,   c->left, a);
    reortho(c);
}

void treegen_turtle_roll(treegen_turtle_state *t, float deg) {
    treegen_turtle *c = &t->cur;
    float a = deg * DEG2RAD;
    c->left = rot_axis(c->left, c->head, a);
    c->up   = rot_axis(c->up,   c->head, a);
    reortho(c);
}

// walk a 3d line from a..b stamping wood. amanatides-ish but we don't need exact
// voxel traversal, just a gap-free chain, so step at sub-cell resolution.
static void wood_line(treegen_buffer *out, block_id id, vec3 a, vec3 b) {
    vec3 d = vec3_sub(b, a);
    float len = vec3_length(d);
    int steps = (int)(len * 2.0f) + 1;     // 2 samples per block, never zero
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / (float)steps;
        vec3 p = vec3_lerp(a, b, t);
        treegen_buffer_add(out, (int)floorf(p.x + 0.5f),
                                (int)floorf(p.y + 0.5f),
                                (int)floorf(p.z + 0.5f), id);
    }
}

void treegen_turtle_forward(treegen_turtle_state *t, treegen_buffer *out,
                            block_id id, int *ox, int *oy, int *oz) {
    treegen_turtle *c = &t->cur;
    vec3 start = c->pos;
    vec3 end   = vec3_add(start, vec3_scale(c->head, c->length));
    c->pos = end;

    if (out && id != BLOCK_AIR)
        wood_line(out, id, start, end);

    if (ox) *ox = (int)floorf(end.x + 0.5f);
    if (oy) *oy = (int)floorf(end.y + 0.5f);
    if (oz) *oz = (int)floorf(end.z + 0.5f);
}
