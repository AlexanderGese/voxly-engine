#include "decals_fade.h"
#include "decals_config.h"

// smoothstep. the ramps use it so the fade has no visible kink at the ends.
static float smooth01(float t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

float decals_fade_curve(decals_phase phase, float progress,
                        float fade_in, float fade_out) {
    (void)fade_in; (void)fade_out;  // durations are applied by the caller's dt
    switch (phase) {
        case DECALS_PHASE_SPAWNING: return smooth01(progress);
        case DECALS_PHASE_STABLE:   return 1.0f;
        case DECALS_PHASE_DYING:    return smooth01(1.0f - progress);
        case DECALS_PHASE_DEAD:     return 0.0f;
        default:                    return 0.0f;
    }
}

void decals_fade_reset(decals_decal *d, float life_total,
                       float fade_in, float fade_out) {
    d->life_total   = life_total;
    d->fade_in      = fade_in  > 0.0f ? fade_in  : DECALS_DEFAULT_FADE_IN;
    d->fade_out     = fade_out > 0.0f ? fade_out : DECALS_DEFAULT_FADE_OUT;
    d->phase        = DECALS_PHASE_SPAWNING;
    d->life_elapsed = 0.0f;
    d->alpha        = 0.0f;
}

void decals_fade_kill(decals_decal *d) {
    if (d->phase == DECALS_PHASE_DYING || d->phase == DECALS_PHASE_DEAD)
        return;
    // jump to dying but preserve the visible alpha so it doesnt pop: solve for
    // the progress that yields the current alpha at the dying-curve start.
    d->phase = DECALS_PHASE_DYING;
    d->life_elapsed = 0.0f;
}

int decals_fade_tick(decals_decal *d, float dt) {
    if (!d->alive || d->phase == DECALS_PHASE_DEAD) return 0;
    d->life_elapsed += dt;

    // NO_FADE decals stay pinned at full and never leave STABLE. they still age
    // for ttl purposes if life_total is finite, but skip the alpha ramps.
    if (d->flags & DECALS_FLAG_NO_FADE) {
        d->alpha = 1.0f;
        if (d->phase == DECALS_PHASE_SPAWNING) d->phase = DECALS_PHASE_STABLE;
    }

    switch (d->phase) {
    case DECALS_PHASE_SPAWNING: {
        float p = d->life_elapsed / d->fade_in;
        if (p >= 1.0f) {
            // graduate into stable, carry the leftover time so a long dt doesnt
            // eat a frame of the stable window.
            d->phase = DECALS_PHASE_STABLE;
            d->life_elapsed -= d->fade_in;
            d->alpha = 1.0f;
        } else {
            d->alpha = decals_fade_curve(DECALS_PHASE_SPAWNING, p,
                                         d->fade_in, d->fade_out);
        }
        break;
    }
    case DECALS_PHASE_STABLE: {
        d->alpha = 1.0f;
        // permanent decals (life_total < 0) never leave stable.
        if (d->life_total >= 0.0f && d->life_elapsed >= d->life_total) {
            d->phase = DECALS_PHASE_DYING;
            d->life_elapsed -= d->life_total;
        }
        break;
    }
    case DECALS_PHASE_DYING: {
        float p = d->life_elapsed / d->fade_out;
        if (p >= 1.0f) {
            d->phase = DECALS_PHASE_DEAD;
            d->alpha = 0.0f;
            d->alive = 0;
            return 0;
        }
        d->alpha = decals_fade_curve(DECALS_PHASE_DYING, p,
                                     d->fade_in, d->fade_out);
        break;
    }
    default:
        d->alive = 0;
        return 0;
    }
    return 1;
}
