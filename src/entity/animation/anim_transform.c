#include "anim_transform.h"
#include "anim_quat.h"

animation_transform animation_transform_identity(void) {
    return (animation_transform){
        .translation = VEC3_ZERO,
        .rotation    = animation_quat_identity(),
        .scale       = vec3_new(1.0f, 1.0f, 1.0f)
    };
}

animation_transform animation_transform_make(vec3 t, animation_quat r, vec3 s) {
    return (animation_transform){.translation = t, .rotation = r, .scale = s};
}

mat4 animation_transform_to_mat4(animation_transform x) {
    // M = T * R * S, hand-folded so we don't do three full mat4_mul for nothing.
    mat4 rot = animation_quat_to_mat4(x.rotation);

    mat4 m;
    // columns 0..2 = rotation basis scaled per-axis
    m.m[0][0] = rot.m[0][0] * x.scale.x;
    m.m[0][1] = rot.m[0][1] * x.scale.x;
    m.m[0][2] = rot.m[0][2] * x.scale.x;
    m.m[0][3] = 0.0f;

    m.m[1][0] = rot.m[1][0] * x.scale.y;
    m.m[1][1] = rot.m[1][1] * x.scale.y;
    m.m[1][2] = rot.m[1][2] * x.scale.y;
    m.m[1][3] = 0.0f;

    m.m[2][0] = rot.m[2][0] * x.scale.z;
    m.m[2][1] = rot.m[2][1] * x.scale.z;
    m.m[2][2] = rot.m[2][2] * x.scale.z;
    m.m[2][3] = 0.0f;

    // translation column
    m.m[3][0] = x.translation.x;
    m.m[3][1] = x.translation.y;
    m.m[3][2] = x.translation.z;
    m.m[3][3] = 1.0f;
    return m;
}

animation_transform animation_transform_combine(animation_transform parent,
                                                animation_transform child) {
    animation_transform out;
    // scale composes component-wise (we only support non-rotated scale; full
    // shear would need a matrix, and nobody's authored that yet)
    out.scale = vec3_mul(parent.scale, child.scale);
    out.rotation = animation_quat_mul(parent.rotation, child.rotation);

    // child translation lives in parent space: scale it, rotate it, offset it.
    vec3 t = vec3_mul(child.translation, parent.scale);
    t = animation_quat_rotate_vec3(parent.rotation, t);
    out.translation = vec3_add(parent.translation, t);
    return out;
}

animation_transform animation_transform_lerp(animation_transform a,
                                             animation_transform b, float t) {
    animation_transform out;
    out.translation = vec3_lerp(a.translation, b.translation, t);
    out.scale       = vec3_lerp(a.scale, b.scale, t);
    // big blends want the constant-velocity arc; transform_lerp is the "big"
    // path (the per-keyframe path uses nlerp directly in the sampler)
    out.rotation    = animation_quat_slerp(a.rotation, b.rotation, t);
    return out;
}

animation_transform animation_transform_add(animation_transform base,
                                            animation_transform add,
                                            animation_transform ref,
                                            float weight) {
    // delta = add relative to ref. translation/scale are simple differences,
    // rotation is the relative quat ref^-1 * add.
    vec3 dt = vec3_sub(add.translation, ref.translation);
    vec3 ds = vec3_sub(add.scale, ref.scale);
    animation_quat dq = animation_quat_mul(
        animation_quat_conjugate(animation_quat_normalize(ref.rotation)),
        add.rotation);

    // scale the delta by weight, then apply to base
    animation_transform out;
    out.translation = vec3_add(base.translation, vec3_scale(dt, weight));
    out.scale       = vec3_add(base.scale, vec3_scale(ds, weight));

    // weighted rotation delta: nlerp from identity toward dq, then stack on base
    animation_quat wq = animation_quat_nlerp(animation_quat_identity(), dq, weight);
    out.rotation = animation_quat_normalize(animation_quat_mul(wq, base.rotation));
    return out;
}
