#include "spawnx_region.h"
#include <string.h>
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
int h = (int)((key ^ (key >> 32)) % SPAWNX_REGION_SLOTS);
for (int probe = 0;
probe < SPAWNX_REGION_SLOTS;
int s = slot_for(m, key, 1);
if (s >= 0) m->slot[s].count++;
}
