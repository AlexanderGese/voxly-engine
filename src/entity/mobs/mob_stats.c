#include "mob_stats.h"

#include <string.h>

// numbers loosely cribbed from the wiki then fudged to feel ok in our world.
static const voxl_mob_spec voxl_mob_specs[VOXL_MOB_KIND_COUNT] = {
    [VOXL_MOB_NONE]     = { "none",     0,  0.0f, 0.0f,  0.0f, 0.0f, 0, 0.0f, 0.0f },
    [VOXL_MOB_ZOMBIE]   = { "zombie",   20, 0.7f, 1.9f, 16.0f, 1.6f, 3, 0.6f, 1.95f },
    [VOXL_MOB_SKELETON] = { "skeleton", 20, 0.7f, 1.7f, 18.0f, 8.0f, 2, 0.6f, 1.99f },
    [VOXL_MOB_CREEPER]  = { "creeper",  20, 0.8f, 1.6f, 14.0f, 2.0f, 0, 0.6f, 1.7f  },
    [VOXL_MOB_SPIDER]   = { "spider",   16, 0.9f, 2.2f, 12.0f, 1.4f, 2, 1.4f, 0.9f  },
    [VOXL_MOB_COW]      = { "cow",      10, 0.6f, 1.4f,  0.0f, 0.0f, 0, 0.9f, 1.3f  },
    [VOXL_MOB_PIG]      = { "pig",      10, 0.6f, 1.3f,  0.0f, 0.0f, 0, 0.9f, 0.9f  },
    [VOXL_MOB_CHICKEN]  = { "chicken",   4, 0.5f, 1.1f,  0.0f, 0.0f, 0, 0.4f, 0.7f  },
    [VOXL_MOB_SHEEP]    = { "sheep",     8, 0.6f, 1.3f,  0.0f, 0.0f, 0, 0.9f, 1.3f  },
};

const voxl_mob_spec *voxl_mob_spec_get(voxl_mob_kind k) {
    if (k < 0 || k >= VOXL_MOB_KIND_COUNT) k = VOXL_MOB_NONE;
    return &voxl_mob_specs[k];
}

void voxl_mob_init(voxl_mob *m, voxl_mob_kind k, vec3 pos, uint32_t id) {
    const voxl_mob_spec *s = voxl_mob_spec_get(k);
    memset(m, 0, sizeof *m);
    m->id         = id;
    m->kind       = k;
    m->state      = VOXL_MS_IDLE;
    m->pos        = pos;
    m->health     = s->max_health;
    m->max_health = s->max_health;
}
