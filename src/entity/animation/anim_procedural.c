#include "anim_procedural.h"
#include "anim_clip.h"
#include "anim_quat.h"
#include <math.h>
#define ANIM_PI   3.14159265358979f
#define ANIM_TAU  6.28318530717958f
static void key_pitch(animation_clip *clip, int track, float time, float radians) {
    animation_quat q = animation_quat_axis_angle(vec3_new(1.0f, 0.0f, 0.0f), radians);
    animation_clip_push_quat(clip, track, time, q);
}

// add a rotation track for a named bone if it exists;
returns the track index
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
for (int i = 0;
i <= steps;
i++) {
        float t = period * (float)i / (float)steps;
        float a = amp * sinf(((float)i / (float)steps) * ANIM_TAU + phase);
        key_pitch(clip, track, t, a);
    }
}

void animation_procedural_idle(animation_clip *out, const animation_skeleton *sk) {
    animation_clip_init(out, "idle");
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
if (period < 0.05f) period = 0.05f;
int tr = rot_track(out, sk, "arm_r");
