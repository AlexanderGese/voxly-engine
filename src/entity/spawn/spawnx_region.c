#include "spawnx_region.h"
#include <string.h>

// open addressing with linear probing. the table is tiny and rarely more than
// a quarter full, so probe chains stay short. key 0 is the empty sentinel; a
// real region key is never 0 because spawnx_region_pack biases both halves.

static int slot_for(spawnx_region_map *m, uint64_t key, int insert) {
    int h = (int)((key ^ (key >> 32)) % SPAWNX_REGION_SLOTS);
    for (int probe = 0; probe < SPAWNX_REGION_SLOTS; probe++) {
        int i = (h + probe) % SPAWNX_REGION_SLOTS;
        if (m->slot[i].key == key) return i;
        if (m->slot[i].key == 0) {
            if (!insert) return -1;
            m->slot[i].key = key;
            m->slot[i].count = 0;
            m->used++;
            return i;
        }
    }
    return -1;   // table full and key absent
}

void spawnx_region_init(spawnx_region_map *m) {
    memset(m, 0, sizeof *m);
}

void spawnx_region_rebuild(spawnx_region_map *m, const mob_registry *mr) {
    memset(m, 0, sizeof *m);
    for (int i = 0; i < mr->count; i++) {
        const entity *e = &mr->list[i];
        if (!e->alive) continue;
        uint64_t key = spawnx_region_pack(spawnx_region_of(e->pos));
        int s = slot_for(m, key, 1);
        if (s >= 0) m->slot[s].count++;
        // if the table is full we just stop tracking new regions; those fall
        // back to "has room" and lean on the global caps instead.
    }
}

int spawnx_region_count(const spawnx_region_map *m, vec3 pos) {
    uint64_t key = spawnx_region_pack(spawnx_region_of(pos));
    // const-correct read: replicate the probe without the insert path.
    int h = (int)((key ^ (key >> 32)) % SPAWNX_REGION_SLOTS);
    for (int probe = 0; probe < SPAWNX_REGION_SLOTS; probe++) {
        int i = (h + probe) % SPAWNX_REGION_SLOTS;
        if (m->slot[i].key == key) return m->slot[i].count;
        if (m->slot[i].key == 0) return 0;
    }
    return 0;
}

int spawnx_region_has_room(const spawnx_region_map *m, vec3 pos) {
    return spawnx_region_count(m, pos) < SPAWNX_REGION_CAP;
}

void spawnx_region_account(spawnx_region_map *m, vec3 pos) {
    uint64_t key = spawnx_region_pack(spawnx_region_of(pos));
    int s = slot_for(m, key, 1);
    if (s >= 0) m->slot[s].count++;
}
