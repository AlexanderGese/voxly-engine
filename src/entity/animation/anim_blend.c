#include "anim_blend.h"
#include "anim_pose.h"

#include <stddef.h>
#include "../../math/easing.h"   // clampf

void animation_blend_node_clip(animation_blend_node *n, animation_sampler *s) {
    n->kind = ANIM_BLEND_CLIP;
    n->sampler = s;
    n->input_count = 0;
    n->param = 0.0f;
    n->ref = NULL;
}

void animation_blend_node_lerp(animation_blend_node *n,
                               animation_blend_node *a,
                               animation_blend_node *b, float param) {
    n->kind = ANIM_BLEND_LERP;
    n->sampler = NULL;
    n->inputs[0] = a;
    n->inputs[1] = b;
    n->input_count = 2;
    n->param = param;
    n->ref = NULL;
}

void animation_blend_node_additive(animation_blend_node *n,
                                   animation_blend_node *base,
                                   animation_blend_node *add,
                                   const animation_pose *ref, float weight) {
    n->kind = ANIM_BLEND_ADDITIVE;
    n->sampler = NULL;
    n->inputs[0] = base;
    n->inputs[1] = add;
    n->input_count = 2;
    n->param = weight;
    n->ref = ref;
}

void animation_blend_node_1d(animation_blend_node *n,
                             animation_blend_node **inputs,
                             const float *thresholds, int count, float coord) {
    if (count > ANIM_BLEND_MAX_INPUTS) count = ANIM_BLEND_MAX_INPUTS;
    n->kind = ANIM_BLEND_1D;
    n->sampler = NULL;
    n->input_count = count;
    n->param = coord;
    n->ref = NULL;
    for (int i = 0; i < count; i++) {
        n->inputs[i] = inputs[i];
        n->thresholds[i] = thresholds[i];
    }
}

// resolve a 1d coordinate to the two bracketing inputs + blend factor. mirrors
// the keyframe bracket logic but over child nodes instead of times.
static void blend1d_bracket(const animation_blend_node *n, float coord,
                            int *lo, int *hi, float *frac) {
    int last = n->input_count - 1;
    if (coord <= n->thresholds[0])    { *lo = *hi = 0;    *frac = 0.0f; return; }
    if (coord >= n->thresholds[last]) { *lo = *hi = last; *frac = 0.0f; return; }

    int i = 0;
    while (i < last && n->thresholds[i + 1] <= coord) i++;
    float a = n->thresholds[i], b = n->thresholds[i + 1];
    float span = b - a;
    *lo = i; *hi = i + 1;
    *frac = (span > 1e-8f) ? (coord - a) / span : 0.0f;
}

void animation_blend_node_eval(animation_blend_node *n,
                               const animation_skeleton *sk,
                               const animation_pose *seed,
                               animation_pose *out_pose) {
    switch (n->kind) {
    case ANIM_BLEND_CLIP: {
        // start from the rest seed so bones the clip skips stay put, then sample
        animation_pose_copy(out_pose, seed);
        if (n->sampler)
            animation_sampler_evaluate(n->sampler, sk, out_pose);
        break;
    }
    case ANIM_BLEND_LERP: {
        float t = clampf(n->param, 0.0f, 1.0f);
        if (t <= 0.0f) { animation_blend_node_eval(n->inputs[0], sk, seed, out_pose); break; }
        if (t >= 1.0f) { animation_blend_node_eval(n->inputs[1], sk, seed, out_pose); break; }
        animation_pose tmp;
        animation_blend_node_eval(n->inputs[0], sk, seed, out_pose);
        animation_blend_node_eval(n->inputs[1], sk, seed, &tmp);
        animation_pose_blend(out_pose, out_pose, &tmp, t);
        break;
    }
    case ANIM_BLEND_ADDITIVE: {
        animation_pose base, add;
        animation_blend_node_eval(n->inputs[0], sk, seed, &base);
        animation_blend_node_eval(n->inputs[1], sk, seed, &add);
        const animation_pose *ref = n->ref ? n->ref : seed;
        animation_pose_add(out_pose, &base, &add, ref, clampf(n->param, 0.0f, 1.0f));
        break;
    }
    case ANIM_BLEND_1D: {
        if (n->input_count == 0) { animation_pose_copy(out_pose, seed); break; }
        int lo, hi; float frac;
        blend1d_bracket(n, n->param, &lo, &hi, &frac);
        if (lo == hi) {
            animation_blend_node_eval(n->inputs[lo], sk, seed, out_pose);
        } else {
            animation_pose tmp;
            animation_blend_node_eval(n->inputs[lo], sk, seed, out_pose);
            animation_blend_node_eval(n->inputs[hi], sk, seed, &tmp);
            animation_pose_blend(out_pose, out_pose, &tmp, frac);
        }
        break;
    }
    default:
        animation_pose_copy(out_pose, seed);  // unknown node, pass the seed
        break;
    }
}
