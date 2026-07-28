#include "ephysics_step.h"
#include "ephysics_sweep.h"
#include "ephysics_aabb.h"

#include <math.h>

static float horiz_dist2(vec3 a, vec3 b) {
    float dx = a.x - b.x, dz = a.z - b.z;
    return dx * dx + dz * dz;
}

float ephysics_settle(const ephys_candidates *c, ephys_body *b, float max_drop) {
    if (max_drop <= 0.0f) return 0.0f;
    // sweep straight down by max_drop, stop on first contact.
    vec3 center = vec3_new(b->pos.x, b->pos.y + b->center_y, b->pos.z);
    vec3 down   = vec3_new(0.0f, -max_drop, 0.0f);
    ephys_hit h = ephysics_sweep(c, center, b->half, down);
    float drop;
    if (h.hit) {
        drop = max_drop * h.t;
        b->flags |= EPHYS_F_GROUNDED;
    } else {
        drop = max_drop;   // nothing to land on; fell the whole way
    }
    b->pos.y -= drop;
    return drop;
}

int ephysics_step(const ephys_candidates *c, ephys_body *b, vec3 delta,
                  vec3 flat_pos) {
    float step = b->mat.step_height;
    if (step <= 0.0f) return 0;

    // only worth stepping if we actually got wedged on a wall this tick.
    if (!(b->flags & (EPHYS_F_WALL_X | EPHYS_F_WALL_Z))) return 0;

    vec3 horiz = vec3_new(delta.x, 0.0f, delta.z);
    if (fabsf(horiz.x) < 1e-6f && fabsf(horiz.z) < 1e-6f) return 0;

    // remember where the body is right now (start of the step attempt)
    vec3 start = b->pos;

    // 1. is there headroom to rise `step`? sweep up and see how far we get.
    vec3 up_center = vec3_new(start.x, start.y + b->center_y, start.z);
    ephys_hit up = ephysics_sweep(c, up_center, b->half,
                                  vec3_new(0.0f, step, 0.0f));
    float rise = up.hit ? step * up.t : step;
    if (rise < 0.05f) return 0;   // basically a ceiling right above us, give up

    // 2. move the body up by `rise`, then redo the horizontal move from there.
    ephys_body trial = *b;
    trial.pos.y = start.y + rise;
    trial.vel   = b->vel;   // keep momentum; resolve will clamp blocked axes
    vec3 leftover = ephysics_resolve(c, &trial, horiz, 3);
    (void)leftover;

    // 3. did stepping get us meaningfully further horizontally than the flat
    // resolve did? if not, the wall is too tall / not a clean ledge.
    float gained_step = horiz_dist2(trial.pos, start);
    float gained_flat = horiz_dist2(flat_pos, start);
    if (gained_step <= gained_flat + 1e-4f) return 0;

    // 4. settle back down onto the ledge. allow dropping up to `rise` plus a
    // smidge so we land flush; never below the original feet level minus
    // skin (else we'd be "stepping" down a cliff).
    ephysics_settle(c, &trial, rise + 0.02f);

    // commit the stepped result.
    b->pos = trial.pos;
    b->vel.x = trial.vel.x;
    b->vel.z = trial.vel.z;
    if (b->vel.y < 0.0f) b->vel.y = 0.0f;   // dont keep downward vel mid-step
    b->flags |= EPHYS_F_STEPPED | EPHYS_F_GROUNDED;
    return 1;
}
