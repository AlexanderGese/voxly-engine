#include "tonemap_exposure.h"

#include <math.h>

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void tonemap_exposure_init(tonemap_exposure *e) {
    e->manual_ev    = TONEMAP_DEFAULT_EV;
    e->auto_ev      = 0.0f;
    e->target_ev    = 0.0f;
    e->auto_enabled = 1;
    e->min_ev       = TONEMAP_MIN_EV;
    e->max_ev       = TONEMAP_MAX_EV;
}

void tonemap_exposure_set_auto(tonemap_exposure *e, int on) {
    e->auto_enabled = on ? 1 : 0;
    if (!e->auto_enabled) {
        // hand control straight back to manual, no easing — feels snappier
        // when a designer flips the switch in the debug ui.
        e->auto_ev   = 0.0f;
        e->target_ev = 0.0f;
    }
}

void tonemap_exposure_set_manual(tonemap_exposure *e, float ev) {
    e->manual_ev = clampf(ev, TONEMAP_MIN_EV, TONEMAP_MAX_EV);
}

void tonemap_exposure_measure(tonemap_exposure *e, float avg_luma) {
    if (!e->auto_enabled) {
        e->target_ev = 0.0f;
        return;
    }

    // clamp the measurement so a degenerate frame cant drag the eye to a wall.
    float luma = clampf(avg_luma, TONEMAP_AE_MIN_LUMA, TONEMAP_AE_MAX_LUMA);

    // we want exposure*luma == middle grey. exposure is 2^ev, so
    // ev = log2(grey / luma). that's the auto component only; manual stacks
    // on top later.
    float ratio = TONEMAP_MIDDLE_GREY / luma;
    float ev    = log2f(ratio);

    e->target_ev = clampf(ev, e->min_ev, e->max_ev);
}

void tonemap_exposure_update(tonemap_exposure *e, float dt) {
    if (!e->auto_enabled) {
        e->auto_ev = 0.0f;
        return;
    }
    if (dt < 0.0f) dt = 0.0f;

    float diff = e->target_ev - e->auto_ev;
    if (fabsf(diff) < 1e-4f) {
        e->auto_ev = e->target_ev;   // snap when basically there
        return;
    }

    // brightening (target above current) adapts faster than darkening, like a
    // real iris closing quicker than it opens.
    float rate = (diff > 0.0f) ? TONEMAP_AE_SPEED_UP : TONEMAP_AE_SPEED_DOWN;
    float step = rate * dt;

    if (step >= fabsf(diff)) {
        e->auto_ev = e->target_ev;
    } else {
        e->auto_ev += (diff > 0.0f) ? step : -step;
    }

    e->auto_ev = clampf(e->auto_ev, e->min_ev, e->max_ev);
}

float tonemap_exposure_ev(const tonemap_exposure *e) {
    float ev = e->manual_ev + (e->auto_enabled ? e->auto_ev : 0.0f);
    return clampf(ev, TONEMAP_MIN_EV, TONEMAP_MAX_EV);
}

float tonemap_exposure_multiplier(const tonemap_exposure *e) {
    return exp2f(tonemap_exposure_ev(e));
}
