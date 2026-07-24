#ifndef ENTITY_ANIMATION_ANIM_TRANSFORM_H
#define ENTITY_ANIMATION_ANIM_TRANSFORM_H

#include "anim_types.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"

// TRS transform helpers. a bone's local transform is one of these; composing
// child*parent walks the hierarchy. we never lerp the matrix form directly —
// that's what the decomposed channels are for.

animation_transform animation_transform_identity(void);
animation_transform animation_transform_make(vec3 t, animation_quat r, vec3 s);

// compose to a column-major matrix: M = T * R * S. point is scaled, then
// rotated, then translated, reading the multiply right-to-left as usual.
mat4 animation_transform_to_mat4(animation_transform x);

// combine parent-space child with its parent's transform. result is the child
// expressed in the parent's parent space. used to flatten the bone chain.
animation_transform animation_transform_combine(animation_transform parent,
                                                animation_transform child);

// per-channel interpolation. translation/scale lerp linearly, rotation slerps.
// this is the building block every blend in the subsystem leans on.
animation_transform animation_transform_lerp(animation_transform a,
                                             animation_transform b, float t);

// additive: take the delta of `add` from its own reference and lay it on top of
// `base`, weighted. used by the additive blend node for hit-reactions etc.
animation_transform animation_transform_add(animation_transform base,
                                            animation_transform add,
                                            animation_transform ref,
                                            float weight);

#endif
