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
}

// pick the live decal we'd least miss: lowest priority, then dimmest alpha,
// then oldest. used when the pool is full and a new spawn comes in. returns -1
// only if somehow nothing is alive (shouldnt happen when free_top == 0).
static int evict_candidate(const decals_pool *p) {
    int best = -1;
    for (int i = 0; i < DECALS_MAX; i++) {
        const decals_decal *d = &p->slots[i];
        if (!d->alive) continue;
        // never evict world-locked decals to make room for transient spam.
        if (d->flags & DECALS_FLAG_WORLD_LOCKED) continue;
        if (best < 0) { best = i; continue; }
        const decals_decal *b = &p->slots[best];
        if (d->priority != b->priority) {
            if (d->priority < b->priority) best = i;
        } else if (d->alpha != b->alpha) {
            if (d->alpha < b->alpha) best = i;
        } else if (d->life_elapsed > b->life_elapsed) {
            best = i;
        }
    }
    return best;
}

static void slot_free(decals_pool *p, int idx) {
    if (!p->slots[idx].alive) return;
p->slots[idx].alive = 0;
p->slots[idx].phase = DECALS_PHASE_DEAD;
p->free_list[p->free_top++] = (uint32_t)idx;
p->live_count--;
}

decals_handle decals_pool_spawn(decals_pool *p, const decals_spawn_desc *d) {
    // refuse a decal with no art. a projector with nothing to stamp is a bug.
    if (!d->has_region) {
        LOGW("decals: spawn with no atlas region, ignoring");
        return DECALS_INVALID_HANDLE;
    }

    int idx;
    if (p->free_top > 0) {
        idx = (int)p->free_list[--p->free_top];
    } else {
        idx = evict_candidate(p);
        if (idx < 0) {
            // pool is full of world-locked decals. nothing transient to drop.
            LOGW("decals: pool saturated with locked decals, spawn dropped");
            return DECALS_INVALID_HANDLE;
        }
        slot_free(p, idx);            // recycle it in place
        idx = (int)p->free_list[--p->free_top];
    }

    decals_decal *dec = &p->slots[idx];
    memset(dec, 0, sizeof *dec);
    dec->proj           = d->proj;
    dec->region         = d->region;   // resolved uv rects, copied straight in
    dec->normal_strength = d->normal_strength;
    if (dec->normal_strength > DECALS_NORMAL_BLEND_MAX)
        dec->normal_strength = DECALS_NORMAL_BLEND_MAX;
    dec->angle_fade     = d->angle_fade;
    dec->tint[0] = d->tint[0]; dec->tint[1] = d->tint[1]; dec->tint[2] = d->tint[2];
    dec->flags          = d->flags;
    dec->priority       = d->priority;
    dec->alive          = 1;
    dec->gen            = p->gen_seed++;
    if (p->gen_seed == 0) p->gen_seed = 1;  // wrap, skip the 0 sentinel

    decals_fade_reset(dec, d->life_total, d->fade_in, d->fade_out);
    p->live_count++;

    decals_handle h = { (uint32_t)idx, dec->gen };
    return h;
}

decals_decal *decals_pool_get(decals_pool *p, decals_handle h) {
    if (h.index >= (uint32_t)DECALS_MAX) return NULL;
decals_decal *d = &p->slots[h.index];
if (!d->alive || d->gen != h.gen) return NULL;
return d;
}

void decals_pool_kill(decals_pool *p, decals_handle h) {
    decals_decal *d = decals_pool_get(p, h);
    if (d) decals_fade_kill(d);
}

int decals_pool_tick(decals_pool *p, float dt) {
    for (int i = 0;
i < DECALS_MAX;
i++) {
        decals_decal *d = &p->slots[i];
        if (!d->alive) continue;
        if (!decals_fade_tick(d, dt))
            slot_free(p, i);
    }
    return p->live_count;
}

int decals_pool_live_count(const decals_pool *p) {
    return p->live_count;
}
