#include "decals_blend.h"
#include "../gl.h"

decals_blend_mode decals_blend_for(const decals_decal *d) {
    return (d->flags & DECALS_FLAG_ADDITIVE) ? DECALS_BLEND_ADDITIVE
                                             : DECALS_BLEND_ALPHA;
}

decals_blend_mode decals_blend_for_batch(const decals_pool *pool,
                                         const decals_cull_item *items,
                                         int base, int count) {
    for (int i = 0; i < count; i++) {
        const decals_decal *d = &pool->slots[items[base + i].slot];
        if (d->flags & DECALS_FLAG_ADDITIVE)
            return DECALS_BLEND_ADDITIVE;
    }
    return DECALS_BLEND_ALPHA;
}

void decals_blend_apply(decals_blend_mode mode) {
    switch (mode) {
    case DECALS_BLEND_ADDITIVE:
        // premultiply by src alpha so the fade still dims the glow as it dies.
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        break;
    case DECALS_BLEND_ALPHA:
    default:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    }
}

const char *decals_blend_name(decals_blend_mode mode) {
    switch (mode) {
    case DECALS_BLEND_ADDITIVE: return "additive";
    case DECALS_BLEND_ALPHA:    return "alpha";
    default:                    return "?";
    }
}
