#include "projectile_ballistic.h"
#include "../../config.h"
#include <math.h>
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
if (disc < 0.0f) return 0;
float sq = sqrtf(disc);
float t0 = (-b - sq) / (2.0f * a);
float t1 = (-b + sq) / (2.0f * a);
t = (t0 > 1e-4f) ? t0 : t1;
if (t1 > 1e-4f && t1 < t) t = t1;
if (t <= 1e-4f) return 0;
}

    if (out_t) *out_t = t;
