#include "ecs_world.h"

#include "../../util/darray.h"
#include "../../util/log.h"

void ecs_world_init(ecs_world *w, uint32_t reserve) {
    ecs_pool_init(&w->pool, reserve);
    for (int c = 0; c < ECS_CMP_COUNT; c++) {
        ecs_store_init(&w->stores[c], ecs_component_size((ecs_cmp)c), 0);
    }
    w->destroy_queue = NULL;
    w->deferring     = 0;
}

void ecs_world_free(ecs_world *w) {
    for (int c = 0; c < ECS_CMP_COUNT; c++)
        ecs_store_free(&w->stores[c]);
    ecs_pool_free(&w->pool);
    darr_free(w->destroy_queue);
}

void ecs_world_clear(ecs_world *w) {
    // cheapest correct path: drop and re-init the stores + pool. they all grow
    // back on demand, so the next batch of spawns reallocs as needed.
    for (int c = 0; c < ECS_CMP_COUNT; c++) {
        size_t sz = w->stores[c].elem_size;
        ecs_store_free(&w->stores[c]);
        ecs_store_init(&w->stores[c], sz, 0);
    }
    uint32_t cap = w->pool.cap;
    ecs_pool_free(&w->pool);
    ecs_pool_init(&w->pool, cap);
    darr_clear(w->destroy_queue);
    w->deferring = 0;
}

ecs_entity ecs_create(ecs_world *w) {
    return ecs_pool_create(&w->pool);
}

// rip the entity out of every store it has a component in, then free the slot.
static void destroy_now(ecs_world *w, ecs_entity e) {
    if (!ecs_pool_alive(&w->pool, e)) return;
    ecs_signature sig = ecs_pool_sig(&w->pool, e);
    // only walk components the signature says are present -- cheaper than
    // poking all eight stores for an entity that has two.
    for (int c = 0; c < ECS_CMP_COUNT; c++) {
        if (ecs_sig_has(sig, (ecs_component_id)c))
            ecs_store_remove(&w->stores[c], e);
    }
    ecs_pool_destroy(&w->pool, e);
}

void ecs_destroy(ecs_world *w, ecs_entity e) {
    if (!ecs_pool_alive(&w->pool, e)) return;
    if (w->deferring) {
        // queue it; flushed after the current tick. dedup isnt worth it, a
        // double-queued entity just no-ops the second time through.
        darr_push(w->destroy_queue, e);
        return;
    }
    destroy_now(w, e);
}

void ecs_flush_destroyed(ecs_world *w) {
    if (!w->destroy_queue) return;
    size_t n = darr_len(w->destroy_queue);
    for (size_t i = 0; i < n; i++)
        destroy_now(w, w->destroy_queue[i]);
    darr_clear(w->destroy_queue);
}

int ecs_alive(const ecs_world *w, ecs_entity e) {
    return ecs_pool_alive(&w->pool, e);
}

uint32_t ecs_count(const ecs_world *w) {
    return w->pool.count;
}

void *ecs_add(ecs_world *w, ecs_entity e, ecs_cmp c, const void *data) {
    if (!ecs_pool_alive(&w->pool, e)) {
        LOGW("ecs: add %s to dead entity %u ignored",
             ecs_component_name(c), ecs_entity_index(e));
        return NULL;
    }
    void *slot = ecs_store_add(&w->stores[c], e, data);
    ecs_signature sig = ecs_sig_set(ecs_pool_sig(&w->pool, e),
                                    (ecs_component_id)c);
    ecs_pool_set_sig(&w->pool, e, sig);
    return slot;
}

void *ecs_get(ecs_world *w, ecs_entity e, ecs_cmp c) {
    if (!ecs_pool_alive(&w->pool, e)) return NULL;
    return ecs_store_get(&w->stores[c], e);
}

int ecs_has(const ecs_world *w, ecs_entity e, ecs_cmp c) {
    if (!ecs_pool_alive(&w->pool, e)) return 0;
    // signature lookup is one bit test vs the store's sparse probe; same answer
    return ecs_sig_has(ecs_pool_sig(&w->pool, e), (ecs_component_id)c);
}

void ecs_remove(ecs_world *w, ecs_entity e, ecs_cmp c) {
    if (!ecs_pool_alive(&w->pool, e)) return;
    ecs_store_remove(&w->stores[c], e);
    ecs_signature sig = ecs_sig_clear(ecs_pool_sig(&w->pool, e),
                                      (ecs_component_id)c);
    ecs_pool_set_sig(&w->pool, e, sig);
}
