#ifndef ENTITY_ANIMATION_ANIM_RIG_H
#define ENTITY_ANIMATION_ANIM_RIG_H

#include "anim_types.h"
#include "anim_skeleton.h"

// canned skeleton builders. the subsystem is data-driven, but we have no asset
// pipeline yet, so mobs get rigged in code. this is where the bone layouts for
// the stock mob shapes live. all dimensions in blocks (the engine's world unit).

// named bone indices for the humanoid rig, so gameplay can grab a hand without
// a string lookup every frame. order matches build order (topological).
typedef enum {
    ANIM_HUMANOID_ROOT = 0,
    ANIM_HUMANOID_HIPS,
    ANIM_HUMANOID_SPINE,
    ANIM_HUMANOID_HEAD,
    ANIM_HUMANOID_ARM_L,
    ANIM_HUMANOID_HAND_L,
    ANIM_HUMANOID_ARM_R,
    ANIM_HUMANOID_HAND_R,
    ANIM_HUMANOID_LEG_L,
    ANIM_HUMANOID_FOOT_L,
    ANIM_HUMANOID_LEG_R,
    ANIM_HUMANOID_FOOT_R,
    ANIM_HUMANOID_BONE_COUNT
} animation_humanoid_bone;

// build the stock biped (player, zombie, skeleton, villager — same skeleton,
// different skins). finalizes the skeleton so it's ready to pose.
void animation_rig_build_humanoid(animation_skeleton *sk);

// build a simple quadruped (cow, pig, sheep): body + four legs + head. fewer
// bones, no spine articulation. also finalized.
void animation_rig_build_quadruped(animation_skeleton *sk);

// fill an upper-body mask (1 for spine/head/arms, 0 for legs/hips) sized to the
// humanoid rig. used by the masked blend so a wave doesn't stop the legs.
void animation_rig_humanoid_upper_mask(unsigned char *mask, int bone_count);

#endif
