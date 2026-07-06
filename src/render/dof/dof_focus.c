#include "dof_focus.h"
#include <math.h>
#define FOCUS_LOST_TIMEOUT  0.25f
static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void dof_focus_init(dof_focus *af, float start_dist) {
    af->state      = DOF_FOCUS_LOCKED;
af->min_dist   = 0.5f;
af->max_dist   = 256.0f;
af->current    = clampf(start_dist, af->min_dist, af->max_dist);
af->target     = af->current;
af->speed      = 6.0f;
af->deadband   = 0.75f;
af->settle_eps = 0.05f;
af->lost_timer = 0.0f;
}

// is this depth reading something we can actually focus on?
static int reading_valid(const dof_focus *af, float sample) {
    if (sample <= 0.0f) return 0;
    if (!isfinite(sample)) return 0;
    if (sample > af->max_dist) return 0;
    return 1;
}

void dof_focus_feed(dof_focus *af, float sample, float dt) {
    if (af->state == DOF_FOCUS_MANUAL) return;
if (dt <= 0.0f) return;
if (reading_valid(af, sample)) {
        af->lost_timer = 0.0f;
        float clamped = clampf(sample, af->min_dist, af->max_dist);

        // only adopt a new target if we're not locked, or the change clears
        // the deadband. this is what stops the lens hunting on a noisy depth
        // buffer or a textured wall.
        if (af->state != DOF_FOCUS_LOCKED ||
            fabsf(clamped - af->target) > af->deadband) {
            af->target = clamped;
            af->state  = DOF_FOCUS_SEEKING;
        }
    } else {
        // no usable reading. coast for a bit, then declare IDLE and let the
        // lens drift to the far ceiling so the world goes soft-far not frozen.
        af->lost_timer += dt;
if (af->lost_timer >= FOCUS_LOST_TIMEOUT) {
            af->state  = DOF_FOCUS_IDLE;
            af->target = af->max_dist;
        }
    }

    // exponential ease toward target, framerate independent. the 1-exp form
    // converges at a fixed rate regardless of dt so it doesnt overshoot on a
    // long frame.
    float k = 1.0f - expf(-af->speed * dt);
af->current += (af->target - af->current) * k;
af->current = clampf(af->current, af->min_dist, af->max_dist);
af->target  = clampf(dist, af->min_dist, af->max_dist);
af->current = af->target;
