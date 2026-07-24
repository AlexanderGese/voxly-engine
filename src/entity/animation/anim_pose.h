#ifndef ENTITY_ANIMATION_ANIM_POSE_H
#define ENTITY_ANIMATION_ANIM_POSE_H

#include "anim_types.h"
#include "anim_skeleton.h"

// pose utilities: the verbs the blend graph and controller use to combine the
// poses samplers hand back. nothing here allocates — poses are fixed-size and
// live on the stack or inside a controller.

// fill every bone with identity TRS. rarely what you want for a real rig (use
// the skeleton rest pose), but handy as a clean slate for additive layers.
void animation_pose_reset(animation_pose *p, int bone_count);

void animation_pose_copy(animation_pose *dst, const animation_pose *src);

// linear cross-fade: out = lerp(a, b, t) per bone. t=0 is all a, t=1 is all b.
// a/b/out may alias; we read before write per bone so in-place is safe.
void animation_pose_blend(animation_pose *out, const animation_pose *a,
                          const animation_pose *b, float t);

// masked blend: only bones whose mask byte is non-zero pull toward b; the rest
// stay at a. this is how you bolt an upper-body wave onto a full-body walk.
// mask has bone_count entries.
void animation_pose_blend_masked(animation_pose *out, const animation_pose *a,
                                 const animation_pose *b, float t,
                                 const unsigned char *mask);

// additive: out = base + weight * (add - ref), per bone. drives hit reactions,
// breathing, recoil — deltas layered on top of whatever's playing underneath.
void animation_pose_add(animation_pose *out, const animation_pose *base,
                        const animation_pose *add, const animation_pose *ref,
                        float weight);

// weighted accumulate for N-way blends: out += weight * pose, finalized by a
// normalize. used by the blend node when more than two inputs are in play.
void animation_pose_accumulate(animation_pose *acc, const animation_pose *p,
                               float weight, int first);
void animation_pose_normalize(animation_pose *acc, float total_weight);

#endif
