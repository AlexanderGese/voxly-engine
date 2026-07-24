#include "anim_sampler.h"
#include "anim_clip.h"

#include <stddef.h>
#include <math.h>

void animation_sampler_init(animation_sampler *s, const animation_clip *clip,
                            int looping) {
    s->clip = clip;
    s->time = 0.0f;
    s->rate = 1.0f;
    s->looping = looping;
    s->finished = 0;
}

void animation_sampler_rewind(animation_sampler *s) {
    s->time = 0.0f;
    s->finished = 0;
}

void animation_sampler_seek_phase(animation_sampler *s, float phase) {
    float dur = s->clip ? s->clip->duration : 0.0f;
    if (phase < 0.0f) phase = 0.0f;
    if (phase > 1.0f) phase = 1.0f;
    s->time = phase * dur;
    s->finished = 0;
}

void animation_sampler_advance(animation_sampler *s, float dt) {
    if (!s->clip) return;
    float dur = s->clip->duration;
    if (dur <= 1e-6f) { s->time = 0.0f; return; }  // static clip, nothing to do

    s->time += dt * s->rate;

    if (s->looping) {
        // fmod can go negative for reverse playback; nudge back into range
        s->time = fmodf(s->time, dur);
        if (s->time < 0.0f) s->time += dur;
    } else {
        if (s->time >= dur) { s->time = dur; s->finished = 1; }
        else if (s->time <= 0.0f) { s->time = 0.0f; }
    }
}

float animation_sampler_phase(const animation_sampler *s) {
    float dur = s->clip ? s->clip->duration : 0.0f;
    if (dur <= 1e-6f) return 0.0f;
    return s->time / dur;
}

void animation_sampler_evaluate(const animation_sampler *s,
                                const animation_skeleton *sk,
                                animation_pose *out_pose) {
    out_pose->bone_count = sk->bone_count;
    const animation_clip *clip = s->clip;
    if (!clip) return;

    // each track patches one channel of one bone's local transform. bones the
    // clip doesn't mention are left untouched (caller seeded them).
    for (int ti = 0; ti < clip->track_count; ti++) {
        const animation_track *t = &clip->tracks[ti];
        if (t->bone < 0 || t->bone >= sk->bone_count) continue;  // stale ref

        animation_transform *xf = &out_pose->locals[t->bone];
        vec3 v; animation_quat q;

        switch (t->kind) {
        case ANIM_CHANNEL_TRANSLATION:
            if (animation_clip_sample_track(t, s->time, &v, NULL))
                xf->translation = v;
            break;
        case ANIM_CHANNEL_SCALE:
            if (animation_clip_sample_track(t, s->time, &v, NULL))
                xf->scale = v;
            break;
        case ANIM_CHANNEL_ROTATION:
            if (animation_clip_sample_track(t, s->time, NULL, &q))
                xf->rotation = q;
            break;
        default:
            break;  // unknown channel, skip rather than corrupt the pose
        }
    }
}
