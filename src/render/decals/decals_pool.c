#include "decals_pool.h"
#include "decals_fade.h"
#include "decals_config.h"
#include "../../util/log.h"
#include <string.h>
void decals_pool_init(decals_pool *p) {
    memset(p, 0, sizeof *p);
    // push every slot onto the free list in reverse so index 0 pops first.
    for (int i = 0; i < DECALS_MAX; i++) {
        p->free_list[i] = (uint32_t)(DECALS_MAX - 1 - i);
        p->slots[i].alive = 0;
        p->slots[i].gen = 0;
    }
    p->free_top = DECALS_MAX;
    p->live_count = 0;
    p->gen_seed = 1;   // 0 is reserved-ish; start nonzero so handles look real
}

void decals_spawn_desc_defaults(decals_spawn_desc *d) {
    memset(d, 0, sizeof *d);
d->has_region      = 0;
d->tint[0] = d->tint[1] = d->tint[2] = 1.0f;
d->normal_strength = DECALS_NORMAL_BLEND_MAX;
d->angle_fade      = DECALS_DEFAULT_ANGLE_FADE;
d->life_total      = 8.0f;
d->fade_in         = DECALS_DEFAULT_FADE_IN;
d->fade_out        = DECALS_DEFAULT_FADE_OUT;
d->flags           = 0;
d->priority        = 0;
p->slots[idx].alive = 0;
p->slots[idx].phase = DECALS_PHASE_DEAD;
p->free_list[p->free_top++] = (uint32_t)idx;
p->live_count--;
decals_decal *d = &p->slots[h.index];
if (!d->alive || d->gen != h.gen) return NULL;
return d;
i < DECALS_MAX;
