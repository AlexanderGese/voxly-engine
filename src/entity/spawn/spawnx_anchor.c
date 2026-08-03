#include "spawnx_anchor.h"
#include <string.h>

void spawnx_anchor_init(spawnx_anchor_store *s) {
    memset(s, 0, sizeof *s);
}

// find the slot holding entity_id, or -1. linear, the store is small.
static int find(const spawnx_anchor_store *s, uint32_t entity_id) {
    if (entity_id == 0) return -1;
    for (int i = 0; i < SPAWNX_ANCHOR_MAX; i++) {
        if (s->anchor[i].entity_id == entity_id) return i;
    }
    return -1;
}

int spawnx_anchor_add(spawnx_anchor_store *s, uint32_t entity_id,
                      spawnx_source reason, int sticky) {
    if (entity_id == 0) return 0;

    int i = find(s, entity_id);
    if (i < 0) {
        // grab a free slot.
        for (int j = 0; j < SPAWNX_ANCHOR_MAX; j++) {
            if (s->anchor[j].entity_id == 0) { i = j; break; }
        }
        if (i < 0) return 0;          // store full
        s->anchor_count++;
    }
    s->anchor[i].entity_id = entity_id;
    s->anchor[i].reason    = (uint8_t)reason;
    s->anchor[i].sticky    = sticky ? 1 : 0;
    return 1;
}

void spawnx_anchor_remove(spawnx_anchor_store *s, uint32_t entity_id) {
    int i = find(s, entity_id);
    if (i < 0) return;
    s->anchor[i].entity_id = 0;
    s->anchor[i].reason = 0;
    s->anchor[i].sticky = 0;
    if (s->anchor_count > 0) s->anchor_count--;
}

int spawnx_anchor_is_held(const spawnx_anchor_store *s, uint32_t entity_id) {
    return find(s, entity_id) >= 0;
}

void spawnx_anchor_sweep(spawnx_anchor_store *s, const mob_registry *mr) {
    for (int i = 0; i < SPAWNX_ANCHOR_MAX; i++) {
        uint32_t id = s->anchor[i].entity_id;
        if (id == 0) continue;
        // is this id still alive in the registry?
        int alive = 0;
        for (int j = 0; j < mr->count; j++) {
            if (mr->list[j].alive && mr->list[j].id == id) { alive = 1; break; }
        }
        if (!alive) {
            s->anchor[i].entity_id = 0;
            s->anchor[i].reason = 0;
            s->anchor[i].sticky = 0;
            if (s->anchor_count > 0) s->anchor_count--;
        }
    }
}

void spawnx_anchor_set_protect(spawnx_anchor_store *s, int idx, vec3 p) {
    if (idx < 0 || idx >= SPAWNX_PROTECT_MAX) return;
    s->protect[idx] = p;
    if (idx + 1 > s->protect_count) s->protect_count = idx + 1;
}

int spawnx_anchor_protected(const spawnx_anchor_store *s, vec3 pos) {
    float r2 = SPAWNX_PROTECT_RADIUS * SPAWNX_PROTECT_RADIUS;
    for (int i = 0; i < s->protect_count; i++) {
        // horizontal distance only; a bed two floors up still protects below it.
        float dx = pos.x - s->protect[i].x;
        float dz = pos.z - s->protect[i].z;
        if (dx * dx + dz * dz < r2) return 1;
    }
    return 0;
}
