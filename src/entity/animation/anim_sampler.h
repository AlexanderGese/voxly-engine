#ifndef ENTITY_ANIMATION_ANIM_SAMPLER_H
#define ENTITY_ANIMATION_ANIM_SAMPLER_H

#include "anim_types.h"
#include "anim_skeleton.h"

// a sampler is a playhead over a clip: it owns the time cursor, loop flag and
// rate, and on demand bakes the clip into a pose. one per active clip on an
// entity; the controller juggles a couple of these.

typedef struct {
    const animation_clip *clip;
    float time;        // current playhead, seconds
    float rate;        // playback multiplier (1 = authored speed, negative ok)
    int   looping;     // wrap at duration vs clamp+hold
    int   finished;    // set when a non-looping clip hits the end
} animation_sampler;

void animation_sampler_init(animation_sampler *s, const animation_clip *clip,
                            int looping);

// reset the playhead to the start and clear the finished flag.
void animation_sampler_rewind(animation_sampler *s);

// jump the playhead to an absolute phase in [0,1] of the clip duration.
void animation_sampler_seek_phase(animation_sampler *s, float phase);

// advance the cursor by dt seconds (scaled by rate) and wrap/clamp per loop.
void animation_sampler_advance(animation_sampler *s, float dt);

// normalized playhead phase in [0,1], handy for syncing two samplers.
float animation_sampler_phase(const animation_sampler *s);

// evaluate the clip at the current cursor into out_pose. bones with no track in
// the clip keep whatever was already in out_pose (so seed it from the rest pose
// or a base layer first). rest is passed so absent channels can fall back.
void animation_sampler_evaluate(const animation_sampler *s,
                                const animation_skeleton *sk,
                                animation_pose *out_pose);

#endif
