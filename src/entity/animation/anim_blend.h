#ifndef ENTITY_ANIMATION_ANIM_BLEND_H
#define ENTITY_ANIMATION_ANIM_BLEND_H

#include "anim_types.h"
#include "anim_skeleton.h"
#include "anim_sampler.h"

// a tiny blend graph. each node either plays a clip (a leaf) or mixes its
// children. evaluating the root produces one pose. it's intentionally small —
// a 1d blendspace (walk<->run keyed on speed) and a couple of mix ops cover
// every mob we have. anything fancier and you'd reach for a real graph editor.

#define ANIM_BLEND_MAX_INPUTS  4

typedef enum {
    ANIM_BLEND_CLIP = 0,   // leaf: sample one clip via its sampler
    ANIM_BLEND_LERP,       // mix two children by `param`
    ANIM_BLEND_ADDITIVE,   // child[0] base + param*(child[1] delta vs ref)
    ANIM_BLEND_1D          // 1d blendspace across N children at sorted thresholds
} animation_blend_kind;

typedef struct animation_blend_node animation_blend_node;
struct animation_blend_node {
    animation_blend_kind kind;

    // leaf payload
    animation_sampler *sampler;     // for ANIM_BLEND_CLIP

    // mix payload
    animation_blend_node *inputs[ANIM_BLEND_MAX_INPUTS];
    float thresholds[ANIM_BLEND_MAX_INPUTS];  // for 1D, ascending
    int   input_count;

    float param;                    // lerp factor / 1d coordinate / add weight

    // additive needs a reference pose (usually the rest pose)
    const animation_pose *ref;
};

void animation_blend_node_clip(animation_blend_node *n, animation_sampler *s);
void animation_blend_node_lerp(animation_blend_node *n,
                               animation_blend_node *a,
                               animation_blend_node *b, float param);
void animation_blend_node_additive(animation_blend_node *n,
                                   animation_blend_node *base,
                                   animation_blend_node *add,
                                   const animation_pose *ref, float weight);

// configure a 1d blendspace. thresholds must be ascending and match inputs 1:1.
void animation_blend_node_1d(animation_blend_node *n,
                             animation_blend_node **inputs,
                             const float *thresholds, int count, float coord);

// evaluate the node (and its subtree) into out_pose. seed must hold the rest
// pose so clip leaves can leave untouched bones alone.
void animation_blend_node_eval(animation_blend_node *n,
                               const animation_skeleton *sk,
                               const animation_pose *seed,
                               animation_pose *out_pose);

#endif
