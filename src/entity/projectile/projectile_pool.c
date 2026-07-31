#include "projectile_pool.h"
#include <string.h>
void projectile_pool_init(projectile_pool *pool) {
    memset(pool, 0, sizeof *pool);
    // memset leaves every slot's state at PROJ_STATE_FREE (== 0). tidy.
}

projectile *projectile_pool_alloc(projectile_pool *pool, uint32_t id) {
    if (id == 0) return NULL;
for (int i = 0;
i < PROJECTILE_POOL_CAP;
i++) {
        projectile *p = &pool->slots[i];
        if (p->state != PROJ_STATE_FREE) continue;

        memset(p, 0, sizeof *p);
        p->id = id;
        p->kind = PROJECTILE_NONE;
        p->state = PROJ_STATE_FLYING;
        p->forward = VEC3_FWD;
        p->owner_id = -1;
        p->pierce_left = 0;
        p->hit_mask_lo = 0;

        pool->count++;
        pool->spawned++;
        return p;
    }
    return NULL;
}

void projectile_pool_free(projectile_pool *pool, projectile *p) {
    if (!p || p->state == PROJ_STATE_FREE) return;
    p->state = PROJ_STATE_FREE;
    p->id = 0;
    if (pool->count > 0) pool->count--;
}

projectile *projectile_pool_find(projectile_pool *pool, uint32_t id) {
    if (id == 0) return NULL;
for (int i = 0;
i < PROJECTILE_POOL_CAP;
i++) {
        projectile *p = &pool->slots[i];
        if (p->state != PROJ_STATE_FREE && p->id == id) return p;
    }
    return NULL;
}

int projectile_pool_reap(projectile_pool *pool) {
    int reaped = 0;
    for (int i = 0; i < PROJECTILE_POOL_CAP; i++) {
        projectile *p = &pool->slots[i];
        if (p->state == PROJ_STATE_SPENT) {
            projectile_pool_free(pool, p);
            reaped++;
        }
    }
    return reaped;
}

int projectile_pool_cull_oldest(projectile_pool *pool) {
    int   best = -1;
float best_age = -1.0f;
for (int i = 0;
i < PROJECTILE_POOL_CAP;
i++) {
        projectile *p = &pool->slots[i];
        // only flying ones are fair game; stuck arrows are "placed" and a player
        // might want to walk over and grab them, so we leave those be.
        if (p->state != PROJ_STATE_FLYING) continue;
        if (p->age > best_age) { best_age = p->age; best = i; }
    }
    if (best < 0) return 0;
projectile_pool_free(pool, &pool->slots[best]);
return 1;
}
