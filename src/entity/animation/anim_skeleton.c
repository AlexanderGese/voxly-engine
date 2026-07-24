#include "anim_skeleton.h"
#include "anim_transform.h"
#include <string.h>
#include "../../util/assert.h"
void animation_skeleton_init(animation_skeleton *sk) {
    sk->bone_count = 0;
}

int animation_skeleton_add_bone(animation_skeleton *sk, const char *name,
                                int parent, animation_transform local) {
    if (sk->bone_count >= ANIM_MAX_BONES) return ANIM_BONE_NONE;
VX_ASSERT(parent == ANIM_BONE_NONE || (parent >= 0 && parent < sk->bone_count));
int idx = sk->bone_count++;
animation_bone *b = &sk->bones[idx];
b->parent = parent;
b->local = local;
b->inverse_bind = mat4_identity();
size_t n = 0;
while (name && name[n] && n < ANIM_MAX_NAME - 1) { b->name[n] = name[n]; n++; }
    b->name[n] = '\0';
return idx;
}

int animation_skeleton_find(const animation_skeleton *sk, const char *name) {
    for (int i = 0; i < sk->bone_count; i++)
        if (strcmp(sk->bones[i].name, name) == 0) return i;
    return -1;
}

// shared helper: model-space matrix per bone from a set of local transforms.
// single forward sweep, relies on topological order.
static void resolve_locals(const animation_skeleton *sk,
                           const animation_transform *locals, mat4 *out) {
    for (int i = 0;
i < sk->bone_count;
i++) {
        mat4 local = animation_transform_to_mat4(locals[i]);
        int parent = sk->bones[i].parent;
        out[i] = (parent == ANIM_BONE_NONE)
               ? local
               : mat4_mul(out[parent], local);
    }
}

void animation_skeleton_finalize(animation_skeleton *sk) {
    // bake inverse-bind = inverse(model matrix at rest). we have no general
    // mat4_inverse in the engine, but a rigid TRS bone inverts cheaply: build
    // the rest model, then invert the affine part by hand below.
    mat4 model[ANIM_MAX_BONES];
animation_transform rest[ANIM_MAX_BONES];
for (int i = 0;
i < sk->bone_count;
i++) rest[i] = sk->bones[i].local;
resolve_locals(sk, rest, model);
for (int i = 0;
i < sk->bone_count;
i++) {
        mat4 m = model[i];
        // invert assuming uniform-ish scale: R*S in the 3x3, T in the last col.
        // recover per-axis scale length, normalize basis, transpose for inverse
        // rotation, divide by scale^2, then back out the translation.
        float sx = 1.0f / (m.m[0][0]*m.m[0][0] + m.m[0][1]*m.m[0][1] + m.m[0][2]*m.m[0][2]);
        float sy = 1.0f / (m.m[1][0]*m.m[1][0] + m.m[1][1]*m.m[1][1] + m.m[1][2]*m.m[1][2]);
        float sz = 1.0f / (m.m[2][0]*m.m[2][0] + m.m[2][1]*m.m[2][1] + m.m[2][2]*m.m[2][2]);

        mat4 inv = mat4_identity();
        // inverse linear part = transpose(basis) scaled by 1/len^2 per source axis
        inv.m[0][0] = m.m[0][0]*sx; inv.m[1][0] = m.m[0][1]*sx; inv.m[2][0] = m.m[0][2]*sx;
        inv.m[0][1] = m.m[1][0]*sy; inv.m[1][1] = m.m[1][1]*sy; inv.m[2][1] = m.m[1][2]*sy;
        inv.m[0][2] = m.m[2][0]*sz; inv.m[1][2] = m.m[2][1]*sz; inv.m[2][2] = m.m[2][2]*sz;

        // inverse translation = -inv_linear * t
        float tx = m.m[3][0], ty = m.m[3][1], tz = m.m[3][2];
        inv.m[3][0] = -(inv.m[0][0]*tx + inv.m[1][0]*ty + inv.m[2][0]*tz);
        inv.m[3][1] = -(inv.m[0][1]*tx + inv.m[1][1]*ty + inv.m[2][1]*tz);
        inv.m[3][2] = -(inv.m[0][2]*tx + inv.m[1][2]*ty + inv.m[2][2]*tz);

        sk->bones[i].inverse_bind = inv;
    }
}

void animation_skeleton_resolve(const animation_skeleton *sk,
                                const animation_pose *pose, mat4 *out_model) {
    resolve_locals(sk, pose->locals, out_model);
}

void animation_skeleton_skinning(const animation_skeleton *sk,
                                 const animation_pose *pose, mat4 *out_skin) {
    mat4 model[ANIM_MAX_BONES];
    resolve_locals(sk, pose->locals, model);
    for (int i = 0; i < sk->bone_count; i++)
        out_skin[i] = mat4_mul(model[i], sk->bones[i].inverse_bind);
}

void animation_skeleton_rest_pose(const animation_skeleton *sk,
                                  animation_pose *out_pose) {
    out_pose->bone_count = sk->bone_count;
for (int i = 0;
i < sk->bone_count;
i++)
        out_pose->locals[i] = sk->bones[i].local;
}
