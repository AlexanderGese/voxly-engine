#include "lt_morph.h"

// classic smoothstep. cheap, monotonic, zero derivative at both ends — exactly
// what you want so a fade doesn't visibly kick at its start or snap at its end.
static float smoothstep01(float t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

void lt_morph_init(lt_morph_state *ms, int level, float speed) {
    ms->from_level = level;
    ms->to_level   = level;
    ms->t          = 1.0f;          // already fully at `to`
    ms->speed      = speed > 0.0f ? speed : 3.0f;   // ~0.33s default
    ms->active     = 0;
}

void lt_morph_retarget(lt_morph_state *ms, int new_level) {
    // already settled there, or already fading toward it — nothing to do.
    if (!ms->active && new_level == ms->to_level) return;
    if (ms->active  && new_level == ms->to_level) return;

    // if a morph was mid-flight to some other level, snap the visual baseline to
    // wherever the fade currently sits and start a fresh one from there. this
    // avoids a hitch when the player wiggles across a band twice quickly.
    if (ms->active) {
        // whichever side we're closer to becomes the new `from`.
        ms->from_level = (ms->t >= 0.5f) ? ms->to_level : ms->from_level;
    } else {
        ms->from_level = ms->to_level;
    }

    ms->to_level = new_level;
    ms->t        = 0.0f;
    ms->active   = 1;
}

int lt_morph_tick(lt_morph_state *ms, float dt) {
    if (!ms->active) return 0;

    ms->t += ms->speed * dt;
    if (ms->t >= 1.0f) {
        ms->t          = 1.0f;
        ms->from_level = ms->to_level;
        ms->active     = 0;
        return 0;
    }
    return 1;
}

float lt_morph_weight(const lt_morph_state *ms) {
    // settled -> fully the target.
    if (!ms->active) return 1.0f;
    return smoothstep01(ms->t);
}

float lt_morph_y_bias(const lt_morph_state *ms, float cell_delta) {
    if (!ms->active) return 0.0f;

    // we only need the sink when coarsening (to_level > from_level): the new
    // coarse cells sit higher than the fine surface they replace, so bias them
    // down by up to half the cell-size delta, eased out as the fade completes.
    if (ms->to_level <= ms->from_level) return 0.0f;

    float w = smoothstep01(ms->t);
    float maxbias = cell_delta * 0.5f;
    return -maxbias * (1.0f - w);   // negative = downward, fades to 0
}
