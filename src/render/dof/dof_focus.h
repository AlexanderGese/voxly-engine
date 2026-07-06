#ifndef RENDER_DOF_FOCUS_H
#define RENDER_DOF_FOCUS_H

#include "dof_coc.h"

// autofocus driver. the lens needs a focus distance every frame; this turns a
// raw "what's under the reticle" depth into a smooth, non-jittery focus_dist
// the way a real af motor would — it eases toward the target, holds when the
// reading is unreliable, and refuses to hunt on tiny depth wobbles.
//
// small state machine:
// IDLE    -> nothing in front, hold last focus (sky, void)
// SEEKING -> target moved enough, easing the lens toward it
// LOCKED  -> settled on the target, only re-seeks past a deadband
// MANUAL  -> external code pinned the distance, af is hands-off

typedef enum {
    DOF_FOCUS_IDLE,
    DOF_FOCUS_SEEKING,
    DOF_FOCUS_LOCKED,
    DOF_FOCUS_MANUAL
} dof_focus_state;

typedef struct {
    dof_focus_state state;
    float current;       // the distance the lens is actually using
    float target;        // distance we're easing toward
    float speed;         // ease rate, world-units per second of error
    float deadband;      // ignore target deltas smaller than this when locked
    float min_dist;      // floor; never focus closer than this
    float max_dist;      // ceiling; readings past this are treated as idle
    float settle_eps;    // |current-target| below this => LOCKED
    float lost_timer;    // seconds we've had no valid reading
} dof_focus;

// init with sane defaults around `start_dist`.
void  dof_focus_init(dof_focus *af, float start_dist);

// feed a raw depth sample (e.g. the depth under the crosshair). pass a
// non-positive or absurd `sample` to signal "no valid reading"; the driver
// then holds and eventually drifts toward max_dist. `dt` is the frame time.
void  dof_focus_feed(dof_focus *af, float sample, float dt);

// pin the focus manually, leaving af mode. dist is clamped to [min,max].
void  dof_focus_set_manual(dof_focus *af, float dist);

// hand control back to the autofocus.
void  dof_focus_resume_auto(dof_focus *af);

// the distance to hand the lens this frame.
float dof_focus_distance(const dof_focus *af);

// copy the live focus distance into a lens so the coc math sees it.
void  dof_focus_apply(const dof_focus *af, dof_lens *lens);

#endif
