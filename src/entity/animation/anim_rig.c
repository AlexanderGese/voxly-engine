#include "anim_rig.h"
#include "anim_transform.h"
#include "anim_quat.h"

// helper: a bone whose local is just an offset from its parent, no rotation.
// most rest poses are T-pose-ish, so identity rotation + a translation covers
// the bulk of it. scale stays unit.
static animation_transform offset(float x, float y, float z) {
    return animation_transform_make(vec3_new(x, y, z),
                                    animation_quat_identity(),
                                    vec3_new(1.0f, 1.0f, 1.0f));
}

void animation_rig_build_humanoid(animation_skeleton *sk) {
    animation_skeleton_init(sk);

    // a ~1.8 block tall biped. measurements are eyeballed off the player box in
    // config.h (PLAYER_HEIGHT 1.8) and look right in-engine; not anatomically
    // precise, but voxels forgive a lot.
    int root  = animation_skeleton_add_bone(sk, "root",  ANIM_BONE_NONE, offset(0.0f, 0.0f, 0.0f));
    int hips  = animation_skeleton_add_bone(sk, "hips",  root,           offset(0.0f, 0.9f, 0.0f));
    int spine = animation_skeleton_add_bone(sk, "spine", hips,           offset(0.0f, 0.45f, 0.0f));
    /* head  */ animation_skeleton_add_bone(sk, "head",  spine,          offset(0.0f, 0.45f, 0.0f));

    // arms hang from the spine, splayed out along x (the "T" in t-pose)
    int arm_l  = animation_skeleton_add_bone(sk, "arm_l",  spine,  offset(-0.3f, 0.35f, 0.0f));
    /* hnd_l*/   animation_skeleton_add_bone(sk, "hand_l", arm_l,  offset(0.0f, -0.55f, 0.0f));
    int arm_r  = animation_skeleton_add_bone(sk, "arm_r",  spine,  offset( 0.3f, 0.35f, 0.0f));
    /* hnd_r*/   animation_skeleton_add_bone(sk, "hand_r", arm_r,  offset(0.0f, -0.55f, 0.0f));

    // legs drop from the hips
    int leg_l  = animation_skeleton_add_bone(sk, "leg_l",  hips,  offset(-0.15f, 0.0f, 0.0f));
    /* ft_l */   animation_skeleton_add_bone(sk, "foot_l", leg_l, offset(0.0f, -0.85f, 0.0f));
    int leg_r  = animation_skeleton_add_bone(sk, "leg_r",  hips,  offset( 0.15f, 0.0f, 0.0f));
    /* ft_r */   animation_skeleton_add_bone(sk, "foot_r", leg_r, offset(0.0f, -0.85f, 0.0f));

    animation_skeleton_finalize(sk);
}

void animation_rig_build_quadruped(animation_skeleton *sk) {
    animation_skeleton_init(sk);

    // body is the root-ish bone; four legs hang off its corners, head off front.
    // shorter and wider than the biped. cows, mostly.
    int body = animation_skeleton_add_bone(sk, "body", ANIM_BONE_NONE, offset(0.0f, 0.7f, 0.0f));
    /* head */ animation_skeleton_add_bone(sk, "head", body, offset(0.0f, 0.1f, 0.6f));

    // legs: front/back x left/right. z sign picks front vs back.
    /* fl */ animation_skeleton_add_bone(sk, "leg_fl", body, offset(-0.3f, -0.7f,  0.5f));
    /* fr */ animation_skeleton_add_bone(sk, "leg_fr", body, offset( 0.3f, -0.7f,  0.5f));
    /* bl */ animation_skeleton_add_bone(sk, "leg_bl", body, offset(-0.3f, -0.7f, -0.5f));
    /* br */ animation_skeleton_add_bone(sk, "leg_br", body, offset( 0.3f, -0.7f, -0.5f));

    animation_skeleton_finalize(sk);
}

void animation_rig_humanoid_upper_mask(unsigned char *mask, int bone_count) {
    // default everything off, then flip on the bones above the waist. if the
    // caller handed us a differently-sized rig we just mask what fits.
    for (int i = 0; i < bone_count; i++) mask[i] = 0;

    static const int upper[] = {
        ANIM_HUMANOID_SPINE, ANIM_HUMANOID_HEAD,
        ANIM_HUMANOID_ARM_L, ANIM_HUMANOID_HAND_L,
        ANIM_HUMANOID_ARM_R, ANIM_HUMANOID_HAND_R
    };
    int n = (int)(sizeof(upper) / sizeof(upper[0]));
    for (int i = 0; i < n; i++)
        if (upper[i] < bone_count) mask[upper[i]] = 1;
}
