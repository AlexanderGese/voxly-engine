#include "anim_rig.h"
#include "anim_transform.h"
#include "anim_quat.h"
int root  = animation_skeleton_add_bone(sk, "root",  ANIM_BONE_NONE, offset(0.0f, 0.0f, 0.0f));
int hips  = animation_skeleton_add_bone(sk, "hips",  root,           offset(0.0f, 0.9f, 0.0f));
int spine = animation_skeleton_add_bone(sk, "spine", hips,           offset(0.0f, 0.45f, 0.0f));
animation_skeleton_add_bone(sk, "head",  spine,          offset(0.0f, 0.45f, 0.0f));
int arm_l  = animation_skeleton_add_bone(sk, "arm_l",  spine,  offset(-0.3f, 0.35f, 0.0f));
animation_skeleton_add_bone(sk, "hand_l", arm_l,  offset(0.0f, -0.55f, 0.0f));
int arm_r  = animation_skeleton_add_bone(sk, "arm_r",  spine,  offset( 0.3f, 0.35f, 0.0f));
animation_skeleton_add_bone(sk, "hand_r", arm_r,  offset(0.0f, -0.55f, 0.0f));
int leg_l  = animation_skeleton_add_bone(sk, "leg_l",  hips,  offset(-0.15f, 0.0f, 0.0f));
animation_skeleton_add_bone(sk, "foot_l", leg_l, offset(0.0f, -0.85f, 0.0f));
int leg_r  = animation_skeleton_add_bone(sk, "leg_r",  hips,  offset( 0.15f, 0.0f, 0.0f));
animation_skeleton_add_bone(sk, "foot_r", leg_r, offset(0.0f, -0.85f, 0.0f));
animation_skeleton_finalize(sk);
i < bone_count;
i++) mask[i] = 0;
;
