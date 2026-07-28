#include "ephysics_impulse.h"
#include <math.h>

void ephysics_impulse_apply(ephys_body *b, vec3 impulse) {
    float m = b->mat.mass;
    if (m <= 1e-4f) return;   // immovable / massless guard
    vec3 dv = vec3_scale(impulse, 1.0f / m);
    b->vel = vec3_add(b->vel, dv);
}

void ephysics_impulse_radial(ephys_body *b, vec3 source, float strength,
                             float radius) {
    if (radius <= 1e-4f) return;

    vec3 away = vec3_sub(b->pos, source);
    float dist = vec3_length(away);

    vec3 dir;
    if (dist < 1e-4f) {
        // dead centre of the blast: no horizontal direction, just launch up.
        dir = VEC3_UP;
        dist = 0.0f;
    } else {
        dir = vec3_scale(away, 1.0f / dist);
    }

    if (dist >= radius) return;          // outside the falloff, nothing to do

    float falloff = 1.0f - (dist / radius);   // 1 at source, 0 at the rim
    float mag = strength * falloff;

    // bias upward a touch so the shove clears the ground and survives friction.
    vec3 impulse = vec3_scale(dir, mag);
    impulse.y += mag * 0.4f;

    ephysics_impulse_apply(b, impulse);
}

vec3 ephysics_impulse_bounce(const ephys_body *b, vec3 vel, vec3 normal,
                             float min_speed) {
    float vn = vec3_dot(vel, normal);
    if (vn >= 0.0f) return vel;   // already moving away from the surface

    // v' = v - (1 + e) * (v·n) * n
    float e = b->mat.restitution;
    vec3 reflected = vec3_sub(vel, vec3_scale(normal, (1.0f + e) * vn));

    // if the bounce is feeble, snap the normal component to zero so we settle
    // instead of buzzing against the surface forever.
    float out = vec3_dot(reflected, normal);
    if (out < min_speed)
        reflected = vec3_sub(reflected, vec3_scale(normal, out));

    return reflected;
}

int ephysics_impulse_clamp(ephys_body *b, float max_speed) {
    float sp = vec3_length(b->vel);
    if (sp <= max_speed || sp < 1e-6f) return 0;
    b->vel = vec3_scale(b->vel, max_speed / sp);
    return 1;
}
