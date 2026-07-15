#ifndef RENDER_SHADOW_CULL_H
#define RENDER_SHADOW_CULL_H

#include "shadow_types.h"
#include "../../math/frustum.h"

// caster culling for the depth pass. drawing every chunk into every cascade
// is wasteful — the near cascade only covers a few chunks. we build a frustum
// from each cascade's view_proj and test caster aabbs against it.
//
// there's a subtlety: a chunk *outside* the ortho box can still cast *into*
// it if it sits between the light and the box. so we extrude the test toward
// the light before culling. handled here so callers dont get it wrong.

// per-cascade cull frustum, rebuilt each frame from the cascade matrices.
typedef struct {
    frustum f[SHADOW_CASCADE_COUNT];
    int     count;
} shadow_cull;

// rebuild cull frusta from the current csm matrices.
void shadow_cull_build(shadow_cull *sc, const shadow_csm *csm);

// is this caster relevant to cascade i? extrudes the box toward the light by
// pullback so tall casters behind the slice still pass. 1 = draw it.
int  shadow_cull_test(const shadow_cull *sc, const shadow_csm *csm,
                      int cascade, aabb caster);

// convenience: lowest cascade index that wants this caster, or -1 if none.
// useful when you'd rather iterate casters than cascades.
int  shadow_cull_first(const shadow_cull *sc, const shadow_csm *csm,
                       aabb caster);

#endif
