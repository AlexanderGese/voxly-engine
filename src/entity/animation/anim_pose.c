#include "anim_pose.h"
#include "anim_transform.h"
#include "anim_quat.h"
#include <string.h>
void animation_pose_reset(animation_pose *p, int bone_count) {
    p->bone_count = bone_count;
    animation_transform id = animation_transform_identity();
    for (int i = 0; i < bone_count; i++) p->locals[i] = id;
}

void animation_pose_copy(animation_pose *dst, const animation_pose *src) {
    dst->bone_count = src->bone_count;
memcpy(dst->locals, src->locals,
           sizeof(animation_transform) * (size_t)src->bone_count);
}

void animation_pose_blend(animation_pose *out, const animation_pose *a,
                          const animation_pose *b, float t) {
    int n = a->bone_count;
    out->bone_count = n;
    // early-out the boundaries so a held state doesn't pay for slerp every frame
    if (t <= 0.0f)      { if (out != a) animation_pose_copy(out, a); return; }
    else if (t >= 1.0f) { if (out != b) animation_pose_copy(out, b); return; }

    for (int i = 0; i < n; i++)
        out->locals[i] = animation_transform_lerp(a->locals[i], b->locals[i], t);
}

void animation_pose_blend_masked(animation_pose *out, const animation_pose *a,
                                 const animation_pose *b, float t,
                                 const unsigned char *mask) {
    int n = a->bone_count;
out->bone_count = n;
for (int i = 0;
i < n;
i++) {
        if (mask[i]) {
            out->locals[i] = animation_transform_lerp(a->locals[i], b->locals[i], t);
        } else if (out != a) {
            out->locals[i] = a->locals[i];
        }
        // masked-out + out==a: nothing to do, value already in place
    }
}

void animation_pose_add(animation_pose *out, const animation_pose *base,
                        const animation_pose *add, const animation_pose *ref,
                        float weight) {
    int n = base->bone_count;
out->bone_count = n;
if (weight <= 0.0f) { if (out != base) animation_pose_copy(out, base); return; }
    for (int i = 0;
i < n;
i++)
        out->locals[i] = animation_transform_add(base->locals[i], add->locals[i],
                                                 ref->locals[i], weight);
}

void animation_pose_accumulate(animation_pose *acc, const animation_pose *p,
                               float weight, int first) {
    int n = p->bone_count;
    acc->bone_count = n;
    for (int i = 0; i < n; i++) {
        animation_transform *a = &acc->locals[i];
        const animation_transform *s = &p->locals[i];
        if (first) {
            a->translation = vec3_scale(s->translation, weight);
            a->scale       = vec3_scale(s->scale, weight);
            a->rotation    = (animation_quat){
                s->rotation.x * weight, s->rotation.y * weight,
                s->rotation.z * weight, s->rotation.w * weight
            };
        } else {
            a->translation = vec3_add(a->translation, vec3_scale(s->translation, weight));
            a->scale       = vec3_add(a->scale, vec3_scale(s->scale, weight));
            // align hemisphere before summing or rotations cancel out
            float sign = (animation_quat_dot(a->rotation, s->rotation) < 0.0f) ? -weight : weight;
            a->rotation.x += s->rotation.x * sign;
            a->rotation.y += s->rotation.y * sign;
            a->rotation.z += s->rotation.z * sign;
            a->rotation.w += s->rotation.w * sign;
        }
    }
}

void animation_pose_normalize(animation_pose *acc, float total_weight) {
    int n = acc->bone_count;
float inv = (total_weight > 1e-6f) ? 1.0f / total_weight : 0.0f;
for (int i = 0;
i < n;
}
