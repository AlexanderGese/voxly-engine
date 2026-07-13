#include "curves.h"

static float clamp01(float t) {
    if (t < 0.0f) return 0.0f;
    if (t > 1.0f) return 1.0f;
    return t;
}

static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

// scalar curve -------------------------------------------------------------

void particles_curve_init(particles_curve *c) {
    c->count = 0;
}

void particles_curve_add(particles_curve *c, float t, float value) {
    if (c->count >= PARTICLES_CURVE_MAX_STOPS) return;  // silently drop extras
    t = clamp01(t);

    // insertion sort by t so eval can walk linearly. tiny n, who cares.
    int i = c->count - 1;
    while (i >= 0 && c->keys[i].t > t) {
        c->keys[i + 1] = c->keys[i];
        i--;
    }
    c->keys[i + 1].t = t;
    c->keys[i + 1].value = value;
    c->count++;
}

float particles_curve_eval(const particles_curve *c, float t) {
    if (c->count == 0) return 1.0f;          // no curve -> identity
    if (c->count == 1) return c->keys[0].value;
    t = clamp01(t);

    if (t <= c->keys[0].t) return c->keys[0].value;
    if (t >= c->keys[c->count - 1].t) return c->keys[c->count - 1].value;

    for (int i = 0; i < c->count - 1; i++) {
        const particles_curve_key *a = &c->keys[i];
        const particles_curve_key *b = &c->keys[i + 1];
        if (t >= a->t && t <= b->t) {
            float span = b->t - a->t;
            float local = span > 1e-6f ? (t - a->t) / span : 0.0f;
            return lerpf(a->value, b->value, local);
        }
    }
    return c->keys[c->count - 1].value;
}

particles_curve particles_curve_const(float value) {
    particles_curve c;
    particles_curve_init(&c);
    particles_curve_add(&c, 0.0f, value);
    return c;
}

particles_curve particles_curve_ramp(float a, float b) {
    particles_curve c;
    particles_curve_init(&c);
    particles_curve_add(&c, 0.0f, a);
    particles_curve_add(&c, 1.0f, b);
    return c;
}

// color gradient -----------------------------------------------------------

static vec4 vmix(vec4 a, vec4 b, float t) {
    vec4 o;
    o.x = lerpf(a.x, b.x, t);
    o.y = lerpf(a.y, b.y, t);
    o.z = lerpf(a.z, b.z, t);
    o.w = lerpf(a.w, b.w, t);
    return o;
}

void particles_gradient_init(particles_gradient *g) {
    g->count = 0;
}

void particles_gradient_add(particles_gradient *g, float t, vec4 color) {
    if (g->count >= PARTICLES_CURVE_MAX_STOPS) return;
    t = clamp01(t);

    int i = g->count - 1;
    while (i >= 0 && g->keys[i].t > t) {
        g->keys[i + 1] = g->keys[i];
        i--;
    }
    g->keys[i + 1].t = t;
    g->keys[i + 1].color = color;
    g->count++;
}

vec4 particles_gradient_eval(const particles_gradient *g, float t) {
    if (g->count == 0) return (vec4){1, 1, 1, 1};
    if (g->count == 1) return g->keys[0].color;
    t = clamp01(t);

    if (t <= g->keys[0].t) return g->keys[0].color;
    if (t >= g->keys[g->count - 1].t) return g->keys[g->count - 1].color;

    for (int i = 0; i < g->count - 1; i++) {
        const particles_grad_key *a = &g->keys[i];
        const particles_grad_key *b = &g->keys[i + 1];
        if (t >= a->t && t <= b->t) {
            float span = b->t - a->t;
            float local = span > 1e-6f ? (t - a->t) / span : 0.0f;
            return vmix(a->color, b->color, local);
        }
    }
    return g->keys[g->count - 1].color;
}
