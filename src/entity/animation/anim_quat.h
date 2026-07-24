#ifndef ENTITY_ANIMATION_ANIM_QUAT_H
#define ENTITY_ANIMATION_ANIM_QUAT_H

#include "anim_types.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"

// quaternion math for bone rotation. minimal: just what the sampler/blender
// actually need. mul is hamilton product (q then p, like matrices read r->l).

animation_quat animation_quat_identity(void);
animation_quat animation_quat_new(float x, float y, float z, float w);

// build from axis-angle. axis need not be unit; we normalize it.
animation_quat animation_quat_axis_angle(vec3 axis, float radians);

animation_quat animation_quat_mul(animation_quat a, animation_quat b);
animation_quat animation_quat_conjugate(animation_quat q);
animation_quat animation_quat_normalize(animation_quat q);
animation_quat animation_quat_neg(animation_quat q);

float          animation_quat_dot(animation_quat a, animation_quat b);
float          animation_quat_length(animation_quat q);

// rotate a vector. cheap-ish; used for sanity checks more than hot paths.
vec3           animation_quat_rotate_vec3(animation_quat q, vec3 v);

// normalized lerp. fast, good enough for adjacent keyframes, takes the short
// arc (flips b if the dot is negative). t in [0,1].
animation_quat animation_quat_nlerp(animation_quat a, animation_quat b, float t);

// spherical lerp. constant angular velocity, slower, used for big blends where
// nlerp's velocity wobble would show. falls back to nlerp when nearly parallel.
animation_quat animation_quat_slerp(animation_quat a, animation_quat b, float t);

// to a column-major rotation mat4 (no translation). matches engine layout.
mat4           animation_quat_to_mat4(animation_quat q);

#endif
