#include "shadow_stabilize.h"

#include <math.h>

void shadow_stabilize_init(shadow_stabilizer *st, float slew_rate) {
    st->smoothed_dir = vec3_new(0.0f, -1.0f, 0.0f);
    st->slew_rate    = slew_rate;
    st->primed       = 0;
}

vec3 shadow_stabilize_dir(shadow_stabilizer *st, vec3 target_dir, float dt) {
    vec3 target = vec3_normalize(target_dir);

    if (!st->primed) {
        st->smoothed_dir = target;
        st->primed = 1;
        return target;
    }

    // exponential slew. alpha = 1 - exp(-rate*dt) is framerate independent,
    // unlike a flat lerp factor which speeds up at high fps.
    float alpha = 1.0f - expf(-st->slew_rate * dt);
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;

    vec3 next = vec3_lerp(st->smoothed_dir, target, alpha);

    // re-normalize; lerp of two unit vectors shortens them slightly.
    float len2 = vec3_length_sq(next);
    if (len2 < 1e-8f) next = target;          // collapsed, snap to target
    else              next = vec3_normalize(next);

    st->smoothed_dir = next;
    return next;
}

float shadow_stabilize_band(const shadow_csm *csm, int split) {
    if (split < 1) split = 1;
    if (split > csm->count - 1) split = csm->count - 1;

    // base the band on the smaller of the two adjacent cascade thicknesses so
    // the fade never eats more than a fraction of either cascade.
    float lo = csm->splits[split]     - csm->splits[split - 1];
    float hi = csm->splits[split + 1] - csm->splits[split];
    float thin = lo < hi ? lo : hi;
    return thin * (SHADOW_FADE_BAND * 0.5f);
}

float shadow_stabilize_blend(const shadow_csm *csm, int split, float view_depth) {
    if (split < 1 || split >= csm->count) return 0.0f;

    float edge = csm->splits[split];
    float band = shadow_stabilize_band(csm, split);
    if (band <= 0.0f) return view_depth >= edge ? 1.0f : 0.0f;

    // smoothstep across [edge-band, edge+band]
    float t = (view_depth - (edge - band)) / (2.0f * band);
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return t * t * (3.0f - 2.0f * t);
}
