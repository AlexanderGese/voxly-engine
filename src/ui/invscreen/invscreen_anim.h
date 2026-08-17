#ifndef UI_INVSCREEN_ANIM_H
#define UI_INVSCREEN_ANIM_H

// small easing helpers for the open/close transition. header-only — these are
// one-liners and inlining them keeps the controller readable. the controller
// stores a raw 0..1 progress and runs it through these to get the scale/fade/
// vertical-slide the panel uses.

// smoothstep ease in/out. the workhorse; gives the panel that soft settle.
static inline float invscreen_ease(float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

// slight overshoot ease-out for the "pop" when it finishes opening. back-ease
// with a fixed tension that looked right; don't crank it or it gets cartoonish.
static inline float invscreen_ease_back(float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    const float k = 1.70158f;
    float u = t - 1.0f;
    return 1.0f + u * u * ((k + 1.0f) * u + k);
}

// panel scale from progress: starts at 0.92 and eases to 1.0 so it grows in
// subtly rather than ballooning from nothing.
static inline float invscreen_anim_scale(float progress) {
    return 0.92f + 0.08f * invscreen_ease(progress);
}

// alpha straight off the eased progress, clamped.
static inline float invscreen_anim_alpha(float progress) {
    return invscreen_ease(progress);
}

// vertical slide in pixels: the panel rises a few px into place as it opens.
static inline float invscreen_anim_offset(float progress) {
    return (1.0f - invscreen_ease(progress)) * 12.0f;
}

#endif
