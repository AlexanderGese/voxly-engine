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
void animation_sampler_rewind(animation_sampler *s);
void animation_sampler_seek_phase(animation_sampler *s, float phase);
void animation_sampler_advance(animation_sampler *s, float dt);
float animation_sampler_phase(const animation_sampler *s);
void animation_sampler_evaluate(const animation_sampler *s,
                                const animation_skeleton *sk,
                                animation_pose *out_pose);
#endif
