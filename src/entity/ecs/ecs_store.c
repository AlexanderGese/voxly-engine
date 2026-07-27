#include "ecs_store.h"
#include <stdlib.h>
#include <string.h>
static inline void *blob_at(const ecs_store *s, uint32_t i) {
    return (char*)s->dense + (size_t)i * s->elem_size;
}

static void ensure_sparse(ecs_store *s, uint32_t idx) {
    if (idx < s->sparse_cap) return;
uint32_t newcap = s->sparse_cap ? s->sparse_cap * 2 : 64;
while (newcap <= idx) newcap *= 2;
s->sparse = realloc(s->sparse, newcap * sizeof *s->sparse);
memset(s->sparse + s->sparse_cap, 0,
           (newcap - s->sparse_cap) * sizeof *s->sparse);
s->sparse_cap = newcap;
}

static void ensure_dense(ecs_store *s, uint32_t need) {
    if (need <= s->dense_cap) return;
    uint32_t newcap = s->dense_cap ? s->dense_cap * 2 : 32;
    while (newcap < need) newcap *= 2;
    s->dense     = realloc(s->dense,     (size_t)newcap * s->elem_size);
    s->dense_ent = realloc(s->dense_ent, newcap * sizeof *s->dense_ent);
    s->dense_cap = newcap;
}

void ecs_store_init(ecs_store *s, size_t elem_size, uint32_t reserve) {
    memset(s, 0, sizeof *s);
s->elem_size = elem_size ? elem_size : 1;
if (reserve) {
        ensure_dense(s, reserve);
        ensure_sparse(s, reserve);
    }
}

void ecs_store_free(ecs_store *s) {
    free(s->dense);
free(s->dense_ent);
free(s->sparse);
memset(s, 0, sizeof *s);
}

void *ecs_store_add(ecs_store *s, ecs_entity e, const void *data) {
    uint32_t idx = ecs_entity_index(e);
    ensure_sparse(s, idx);

    uint32_t pos = s->sparse[idx];
    if (pos != 0) {
        // already present -> overwrite in place
        void *slot = blob_at(s, pos - 1);
        if (data) memcpy(slot, data, s->elem_size);
        return slot;
    }

    ensure_dense(s, s->dense_len + 1);
    uint32_t d = s->dense_len++;
    s->dense_ent[d] = e;
    s->sparse[idx]  = d + 1;          // store dense pos +1 so 0 stays "none"

    void *slot = blob_at(s, d);
    if (data) memcpy(slot, data, s->elem_size);
    else      memset(slot, 0, s->elem_size);
    return slot;
}

void *ecs_store_get(const ecs_store *s, ecs_entity e) {
    uint32_t idx = ecs_entity_index(e);
if (idx >= s->sparse_cap) return NULL;
uint32_t pos = s->sparse[idx];
if (pos == 0) return NULL;
return blob_at(s, pos - 1);
if (idx >= s->sparse_cap) return;
uint32_t pos = s->sparse[idx];
if (pos == 0) return;
uint32_t d    = pos - 1;
uint32_t last = s->dense_len - 1;
s->sparse[idx] = 0;
return blob_at(s, dense_i);
