#include "anim_procedural.h"
#include "anim_clip.h"
#include "anim_quat.h"

#include <math.h>

// engine builds strict c11, so no M_PI. local constants, like dnc_math.h does.
#define ANIM_PI   3.14159265358979f
#define ANIM_TAU  6.28318530717958f

// rotation key about the x axis (limb fore/aft swing) at `time`. the x axis is
// the natural hinge for arms/legs on the humanoid rig (they hang along -y).
static void key_pitch(animation_clip *clip, int track, float time, float radians) {
    animation_quat q = animation_quat_axis_angle(vec3_new(1.0f, 0.0f, 0.0f), radians);
    animation_clip_push_quat(clip, track, time, q);
}

// add a rotation track for a named bone if it exists; returns the track index
// or -1 when the rig doesn't have that bone (so callers can skip cleanly).
static int rot_track(animation_clip *clip, const animation_skeleton *sk,
                     const char *bone) {
    int b = animation_skeleton_find(sk, bone);
    if (b < 0) return -1;
    return animation_clip_add_track(clip, b, ANIM_CHANNEL_ROTATION);
}

// sample a sine swing into `steps` keys across one period and write them to a
// pitch track. phase shifts the wave so opposite limbs counter-swing.
static void bake_swing(animation_clip *clip, int track, float period,
                       float amp, float phase, int steps) {
    if (track < 0) return;
    for (int i = 0; i <= steps; i++) {
        float t = period * (float)i / (float)steps;
        float a = amp * sinf(((float)i / (float)steps) * ANIM_TAU + phase);
        key_pitch(clip, track, t, a);
    }
}

void animation_procedural_idle(animation_clip *out, const animation_skeleton *sk) {
    animation_clip_init(out, "idle");

    // a slow breathing bob on the spine: tiny vertical translation, ~4s loop.
    int spine = animation_skeleton_find(sk, "spine");
    if (spine >= 0) {
        int tr = animation_clip_add_track(out, spine, ANIM_CHANNEL_TRANSLATION);
        const int steps = 8;
        const float period = 4.0f;
        // keep the bone's rest x/z; only nudge y. rest spine sits at +0.45 over
        // hips on the humanoid rig, so we breathe around that.
        float base_y = sk->bones[spine].local.translation.y;
        for (int i = 0; i <= steps; i++) {
            float t = period * (float)i / (float)steps;
            float dy = 0.02f * sinf(((float)i / (float)steps) * ANIM_TAU);
            animation_clip_push_vec(out, tr, t,
                vec3_new(sk->bones[spine].local.translation.x, base_y + dy,
                         sk->bones[spine].local.translation.z));
        }
    }

    animation_clip_finalize(out);
}

void animation_procedural_walk(animation_clip *out, const animation_skeleton *sk,
                               float swing, float period) {
    animation_clip_init(out, "walk");
    if (period < 0.1f) period = 0.1f;   // guard against a divide-by-tiny later
    const int steps = 12;

    // legs swing in antiphase; arms counter the legs (left arm with right leg).
    // phases are pi apart for the obvious diagonal gait.
    bake_swing(out, rot_track(out, sk, "leg_l"), period, swing, 0.0f, steps);
    bake_swing(out, rot_track(out, sk, "leg_r"), period, swing, ANIM_PI, steps);
    // arms swing a touch less than legs and opposite their diagonal partner
    bake_swing(out, rot_track(out, sk, "arm_l"), period, swing * 0.7f, ANIM_PI, steps);
    bake_swing(out, rot_track(out, sk, "arm_r"), period, swing * 0.7f, 0.0f, steps);

    // quadruped fallback: if the biped bones were absent, swing the four legs.
    bake_swing(out, rot_track(out, sk, "leg_fl"), period, swing, 0.0f, steps);
    bake_swing(out, rot_track(out, sk, "leg_fr"), period, swing, ANIM_PI, steps);
    bake_swing(out, rot_track(out, sk, "leg_bl"), period, swing, ANIM_PI, steps);
    bake_swing(out, rot_track(out, sk, "leg_br"), period, swing, 0.0f, steps);

    animation_clip_finalize(out);
}

void animation_procedural_attack(animation_clip *out, const animation_skeleton *sk,
                                 float period) {
    animation_clip_init(out, "attack");
    if (period < 0.05f) period = 0.05f;

    // a fast overhead-ish swing on the right arm: wind back, snap forward, ease
    // home. three keyframes is enough to read as a strike at mob scale.
    int tr = rot_track(out, sk, "arm_r");
    if (tr >= 0) {
        key_pitch(out, tr, 0.0f,            0.0f);          // neutral
        key_pitch(out, tr, period * 0.25f,  0.9f);          // wind back (up)
        key_pitch(out, tr, period * 0.55f, -1.6f);          // strike down/forward
        key_pitch(out, tr, period,          0.0f);          // recover
    }

    animation_clip_finalize(out);
}

void animation_procedural_hurt(animation_clip *out, const animation_skeleton *sk) {
    animation_clip_init(out, "hurt");

    // a short spine recoil meant to be layered additively. small angle, ~0.3s,
    // back to neutral so the additive delta resolves to zero at the ends.
    int tr = rot_track(out, sk, "spine");
    if (tr >= 0) {
        key_pitch(out, tr, 0.0f,  0.0f);
        key_pitch(out, tr, 0.08f, 0.35f);   // jerk back
        key_pitch(out, tr, 0.30f, 0.0f);    // settle
    }

    animation_clip_finalize(out);
}
