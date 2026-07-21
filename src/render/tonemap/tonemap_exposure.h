#ifndef RENDER_TONEMAP_EXPOSURE_H
#define RENDER_TONEMAP_EXPOSURE_H
#include "tonemap_config.h"
// exposure state. holds the manual ev bias plus the slowly-adapting
// auto-exposure value, so the camera eases between bright caves and bright
// snowfields instead of clipping the instant you walk outside.
//
// the model: scene gives us an average luma (we get it from a downsampled
// readback or a guess). we compute the ev that would land middle-grey on
// that luma, then ease our current ev toward it at a stop-per-second rate.
// final linear multiplier = 2^(manual_ev + auto_ev).
typedef struct {
    float manual_ev;    // user/designer bias, in stops
    float auto_ev;      // current adapted exposure, in stops
    float target_ev;    // where auto_ev is heading
    int   auto_enabled; // 0 = pure manual
    float min_ev;       // adaptation clamp
    float max_ev;
} tonemap_exposure;
// init to neutral: 0 ev, auto on, default clamps.
void  tonemap_exposure_init(tonemap_exposure *e);
// turn auto-exposure on/off. when off, target/auto are pinned to manual.
void  tonemap_exposure_set_auto(tonemap_exposure *e, int on);
// set the manual ev bias, clamped to the configured stop range.
void  tonemap_exposure_set_manual(tonemap_exposure *e, float ev);
// feed the latest measured average scene luminance. recomputes target_ev.
// luma is clamped to a sane window first so a single black/white frame cant
// yank the eye to an extreme.
void  tonemap_exposure_measure(tonemap_exposure *e, float avg_luma);
// step the adaptation by dt seconds. eases auto_ev toward target_ev using an
// asymmetric rate (faster when brightening). no-op when auto is disabled
// beyond pinning to manual.
void  tonemap_exposure_update(tonemap_exposure *e, float dt);
// the linear multiplier to hand the tonemap curve / shader: 2^(manual+auto).
float tonemap_exposure_multiplier(const tonemap_exposure *e);
// the effective ev (manual + auto), for debug readouts.
float tonemap_exposure_ev(const tonemap_exposure *e);
#endif
