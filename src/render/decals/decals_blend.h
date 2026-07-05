#ifndef RENDER_DECALS_BLEND_H
#define RENDER_DECALS_BLEND_H

#include "decals_types.h"
#include "decals_pool.h"
#include "decals_cull.h"

// blend-mode resolution for the stamp pass. a decal's flags decide how its
// albedo combines with whatever's already in the g-buffer: plain alpha for
// blood/footprints, additive for glowing scorch/runes. the normal + material
// outputs always alpha-blend regardless — only the color attachment changes,
// which we cant express per-attachment in plain gl3.3, so the pass picks the
// dominant mode per batch and the cull sort keeps additive decals clustered.

typedef enum {
    DECALS_BLEND_ALPHA = 0,   // src.a over dst — the default
    DECALS_BLEND_ADDITIVE,    // src.rgb * src.a added to dst — glow
} decals_blend_mode;

// derive the blend mode a decal wants from its flags.
decals_blend_mode decals_blend_for(const decals_decal *d);

// derive the mode for a whole batch: the cull items [base, base+count) indexing
// into the pool. an additive decal anywhere in the batch promotes the batch to
// additive (glow wins over a few mis-blended alpha splats, the lesser evil).
decals_blend_mode decals_blend_for_batch(const decals_pool *pool,
                                         const decals_cull_item *items,
                                         int base, int count);

// push the gl blend func for the given mode. assumes GL_BLEND is already on.
void decals_blend_apply(decals_blend_mode mode);

// human-readable name, for the debug overlay.
const char *decals_blend_name(decals_blend_mode mode);

#endif
