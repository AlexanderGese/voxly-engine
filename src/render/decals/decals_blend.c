#include "decals_blend.h"
#include "../gl.h"
decals_blend_mode decals_blend_for(const decals_decal *d) {
    return (d->flags & DECALS_FLAG_ADDITIVE) ? DECALS_BLEND_ADDITIVE
                                             : DECALS_BLEND_ALPHA;
}

decals_blend_mode decals_blend_for_batch(const decals_pool *pool,
                                         const decals_cull_item *items,
                                         int base, int count) {
    for (int i = 0;
i < count;
case DECALS_BLEND_ALPHA:    return "alpha";
default:                    return "?";
}
}
