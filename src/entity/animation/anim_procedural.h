#ifndef ENTITY_ANIMATION_ANIM_PROCEDURAL_H
#define ENTITY_ANIMATION_ANIM_PROCEDURAL_H

#include "anim_types.h"
#include "anim_skeleton.h"

// procedural clip bakers. with no asset pipeline, the canned mob clips are
// synthesized here: a couple of sine-driven keyframe tracks per limb. it's the
// same swing the old procedural animator did, except baked into real clips so
// they can flow through the blend graph and cross-fade like authored data.
//
// all of these allocate tracks via the clip module; free with animation_clip_free.

// a gentle idle: subtle breathing bob on the spine, near-still limbs. loops.
void animation_procedural_idle(animation_clip *out, const animation_skeleton *sk);

// a walk cycle. limbs counter-swing; `swing` is the peak limb angle in radians
// and `period` the seconds per full stride. tuned for the humanoid rig but it
// degrades gracefully on any rig with matching bone names.
void animation_procedural_walk(animation_clip *out, const animation_skeleton *sk,
                               float swing, float period);

// a one-shot attack swing on the right arm. non-looping; duration ~= period.
void animation_procedural_attack(animation_clip *out, const animation_skeleton *sk,
                                 float period);

// a quick additive hurt twitch: a brief spine recoil. meant to be layered
// additively over whatever's playing, so keep it short and small.
void animation_procedural_hurt(animation_clip *out, const animation_skeleton *sk);

#endif
