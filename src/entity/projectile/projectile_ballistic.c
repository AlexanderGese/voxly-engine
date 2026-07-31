#include "projectile_ballistic.h"

#include "../../config.h"
#include <math.h>

// drag model is linear in velocity (v' = -k v). cheap, stable, and for arrow
// speeds it reads fine. a proper v^2 model just makes the tuning harder and the
// step less stable at big dt, and nobody's clocking the reynolds number of a
// minecraft arrow.

void projectile_ballistic_step(projectile *p, float dt, vec3 *out_next) {
    const projectile_def *d = projectile_kind_def(p->kind);

    vec3 a = VEC3_ZERO;

    if (!d->gravity_immune) {
        // GRAVITY is negative already (see config.h). scale per kind.
        a.y += GRAVITY * d->gravity_scale;
    }

    // linear drag: acceleration opposing velocity, magnitude k*|v|.
    if (d->drag > 0.0f) {
        a = vec3_add(a, vec3_scale(p->vel, -d->drag));
    }

    // semi-implicit euler: integrate velocity first, then position with the new
    // velocity. more stable than plain euler for the springy drag term.
    p->vel = vec3_add(p->vel, vec3_scale(a, dt));

    // clamp the fall so a long drop doesnt tunnel through the floor between
    // steps. matches the rest of the engine using TERMINAL_VELOCITY.
    if (p->vel.y < TERMINAL_VELOCITY) p->vel.y = TERMINAL_VELOCITY;

    // keep a heading around for render orientation + tip math. below a crawl we
    // freeze it so a near-stationary arrow doesnt spin to face gravity.
    float sp = vec3_length(p->vel);
    if (sp > 0.5f) {
        p->forward = vec3_scale(p->vel, 1.0f / sp);
    }

    *out_next = vec3_add(p->pos, vec3_scale(p->vel, dt));
}

int projectile_ballistic_predict(vec3 origin, vec3 vel, float gravity,
                                 float ground_y, vec3 *out_hit, float *out_t) {
    // solve origin.y + vy t + 0.5 g t^2 = ground_y for the later positive root.
    float a = 0.5f * gravity;
    float b = vel.y;
    float c = origin.y - ground_y;

    float t;
    if (fabsf(a) < 1e-6f) {
        // no gravity: straight line. only descends if vy < 0.
        if (vel.y >= -1e-6f) return 0;
        t = -c / b;
    } else {
        float disc = b * b - 4.0f * a * c;
        if (disc < 0.0f) return 0;        // arc never reaches the plane
        float sq = sqrtf(disc);
        float t0 = (-b - sq) / (2.0f * a);
        float t1 = (-b + sq) / (2.0f * a);
        // pick the smallest strictly-positive root.
        t = (t0 > 1e-4f) ? t0 : t1;
        if (t1 > 1e-4f && t1 < t) t = t1;
        if (t <= 1e-4f) return 0;
    }

    if (out_t) *out_t = t;
    if (out_hit) {
        *out_hit = (vec3){
            origin.x + vel.x * t,
            origin.y + vel.y * t + 0.5f * gravity * t * t,
            origin.z + vel.z * t,
        };
    }
    return 1;
}

int projectile_ballistic_solve_pitch(float d, float dy, float v, float g,
                                     float *out_pitch) {
    // standard projectile range equation solved for theta:
    // tan(theta) = (v^2 - sqrt(v^4 - g(g d^2 + 2 dy v^2))) / (g d)
    // g here is the *magnitude* of gravity (positive). guard the discriminant.
    if (d <= 1e-4f || v <= 1e-4f) return 0;
    float g_mag = fabsf(g);
    float v2 = v * v;
    float root = v2 * v2 - g_mag * (g_mag * d * d + 2.0f * dy * v2);
    if (root < 0.0f) return 0;            // target unreachable at this speed
    float pitch = atanf((v2 - sqrtf(root)) / (g_mag * d));
    if (out_pitch) *out_pitch = pitch;
    return 1;
}
