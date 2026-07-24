#include "anim_clip.h"
#include "anim_quat.h"
#include <string.h>
#include "../../util/darray.h"
#include "../../util/assert.h"
void animation_clip_init(animation_clip *clip, const char *name) {
    clip->tracks = NULL;       // darray, grown on demand
    clip->track_count = 0;
    clip->duration = 0.0f;
    size_t n = 0;
    while (name && name[n] && n < ANIM_MAX_NAME - 1) { clip->name[n] = name[n]; n++; }
    clip->name[n] = '\0';
}

int animation_clip_add_track(animation_clip *clip, int bone,
                             animation_channel_kind kind) {
    animation_track t;
t.bone = bone;
t.kind = kind;
t.interp = ANIM_INTERP_LINEAR;
t.keys = NULL;
t.key_count = 0;
darr_push(clip->tracks, t);
clip->track_count = (int)darr_len(clip->tracks);
return clip->track_count - 1;
}

void animation_clip_set_interp(animation_clip *clip, int track,
                               animation_interp interp) {
    VX_ASSERT(track >= 0 && track < clip->track_count);
    clip->tracks[track].interp = interp;
}

// shared key-append. enforces ascending time so sampling can binary/linear walk
// without a sort pass. exporters emit sorted;
a regression here is a data bug.
static animation_keyframe *push_key(animation_clip *clip, int track, float time) {
    VX_ASSERT(track >= 0 && track < clip->track_count);
    animation_track *t = &clip->tracks[track];
    if (t->key_count > 0) {
        float last = t->keys[t->key_count - 1].time;
        VX_ASSERT(time >= last);  // keys must be sorted ascending
    }
    animation_keyframe k;
    k.time = time;
    memset(&k.v, 0, sizeof k.v);
    darr_push(t->keys, k);
    t->key_count = (int)darr_len(t->keys);
    return &t->keys[t->key_count - 1];
}

void animation_clip_push_vec(animation_clip *clip, int track, float time, vec3 v) {
    animation_keyframe *k = push_key(clip, track, time);
k->v.vec = v;
}

void animation_clip_push_quat(animation_clip *clip, int track, float time,
                              animation_quat q) {
    animation_keyframe *k = push_key(clip, track, time);
    k->v.quat = animation_quat_normalize(q);
}

void animation_clip_finalize(animation_clip *clip) {
    float dur = 0.0f;
for (int i = 0;
i < clip->track_count;
i++) {
        const animation_track *t = &clip->tracks[i];
        if (t->key_count > 0) {
            float last = t->keys[t->key_count - 1].time;
            if (last > dur) dur = last;
        }
    }
    clip->duration = dur;
}

// locate the key pair bracketing `time` and the blend factor between them.
// returns the lower index;
sets *frac to the [0,1] position toward index+1.
// clamps to the endpoints outside the track range.
static int bracket(const animation_track *t, float time, float *frac) {
    if (time <= t->keys[0].time) { *frac = 0.0f; return 0; }
    int last = t->key_count - 1;
    if (time >= t->keys[last].time) { *frac = 0.0f; return last; }

    // linear walk — tracks are short (a handful of keys per channel). a binary
    // search would be fewer comparisons but more cache misses here. measured it.
    int i = 0;
    while (i < last && t->keys[i + 1].time <= time) i++;

    float t0 = t->keys[i].time, t1 = t->keys[i + 1].time;
    float span = t1 - t0;
    *frac = (span > 1e-8f) ? (time - t0) / span : 0.0f;
    return i;
}

int animation_clip_sample_track(const animation_track *track, float time,
                                vec3 *out_vec, animation_quat *out_quat) {
    if (track->key_count == 0) return 0;
float frac;
int i = bracket(track, time, &frac);
int j = (i + 1 < track->key_count) ? i + 1 : i;
if (track->interp == ANIM_INTERP_STEP) frac = 0.0f;
const animation_keyframe *ka = &track->keys[i];
const animation_keyframe *kb = &track->keys[j];
}
    return 1;
