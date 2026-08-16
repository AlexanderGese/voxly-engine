#ifndef UI_HUD2_ANIM_H
#define UI_HUD2_ANIM_H

// small easing + smoothing helpers shared by the hud2 widgets. nothing
// fancy, just enough to make bars and pops feel alive instead of snapping.

#include <math.h>

static inline float hud2_clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static inline float hud2_lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

// framerate-independent exponential approach. rate is "how fast", higher snaps
// quicker. derived so dt doesnt make it overshoot at low fps.
static inline float hud2_approach(float cur, float target, float rate, float dt) {
    float t = 1.0f - expf(-rate * dt);
    return cur + (target - cur) * t;
}

// classic smoothstep, t in 0..1.
static inline float hud2_smoothstep(float t) {
    t = hud2_clampf(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// overshoot ease for "pop" effects (back-out). c controls how much it
// overshoots before settling. t in 0..1.
static inline float hud2_ease_back(float t, float c) {
    t = hud2_clampf(t, 0.0f, 1.0f);
    float s = t - 1.0f;
    return 1.0f + s * s * ((c + 1.0f) * s + c);
}

// a 0..1 -> 0..1 hump (up then back down). good for one-shot pulses.
static inline float hud2_pulse(float t) {
    t = hud2_clampf(t, 0.0f, 1.0f);
    return sinf(t * 3.14159265f);
}

#endif
