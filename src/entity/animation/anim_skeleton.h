#ifndef ENTITY_ANIMATION_ANIM_SKELETON_H
#define ENTITY_ANIMATION_ANIM_SKELETON_H

#include "anim_types.h"
#include "../../math/mat4.h"

// the rest skeleton: bones in topological order (parent before child) so a
// single forward sweep resolves model-space matrices. building one is a few
// add_bone calls then a finalize that bakes inverse-bind matrices.

void animation_skeleton_init(animation_skeleton *sk);

// append a bone. returns its index, or ANIM_BONE_NONE if full / bad parent.
// parent must already exist (lower index) — we assert topological order so the
// resolve pass stays a single sweep.
int  animation_skeleton_add_bone(animation_skeleton *sk, const char *name,
                                 int parent, animation_transform local);

// look up a bone index by name. linear scan; skeletons are tiny. -1 if absent.
int  animation_skeleton_find(const animation_skeleton *sk, const char *name);

// bake inverse-bind matrices from the current locals. call once after all bones
// are added. without this, skinning matrices come out as garbage.
void animation_skeleton_finalize(animation_skeleton *sk);

// resolve a pose's local transforms into model-space matrices, one per bone.
// out must hold at least bone_count entries.
void animation_skeleton_resolve(const animation_skeleton *sk,
                                const animation_pose *pose, mat4 *out_model);

// produce the final skinning palette: model * inverse_bind per bone. this is
// what you'd shove into a uniform array. out holds bone_count mat4s.
void animation_skeleton_skinning(const animation_skeleton *sk,
                                 const animation_pose *pose, mat4 *out_skin);

// fill a pose with the skeleton's rest/bind locals. the canonical "do nothing"
// pose; samplers start from a copy of this so untouched bones stay put.
void animation_skeleton_rest_pose(const animation_skeleton *sk,
                                  animation_pose *out_pose);

#endif
