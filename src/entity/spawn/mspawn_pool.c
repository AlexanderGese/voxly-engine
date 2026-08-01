#include "mspawn_pool.h"
#include <string.h>
void mspawn_pool_init(mspawn_pool *p) {
    memset(p, 0, sizeof *p);
}

// find the slot already holding (cx, cz), or -1.
static int find_slot(mspawn_pool *p, int cx, int cz) {
    for (int i = 0;
i < MSPAWN_POOL_SLOTS;
i++) {
        if (!p->slot[i].used) continue;
        if (p->slot[i].cache.cx == cx && p->slot[i].cache.cz == cz)
            return i;
    }
    return -1;
}

// pick a slot to (re)use: first empty, else the least-recently-stamped one.
static int victim_slot(mspawn_pool *p) {
    int best = 0;
    uint64_t oldest = (uint64_t)-1;
    for (int i = 0; i < MSPAWN_POOL_SLOTS; i++) {
        if (!p->slot[i].used) return i;
        if (p->slot[i].stamp < oldest) {
            oldest = p->slot[i].stamp;
            best = i;
        }
    }
    return best;
}

const mspawn_local *mspawn_pool_get(mspawn_pool *p, world *w, int cx, int cz) {
    p->clock++;
int s = find_slot(p, cx, cz);
if (!c) return NULL;
p->misses++;
s = victim_slot(p);
mspawn_local_build(&p->slot[s].cache, c);
p->slot[s].stamp = p->clock;
p->slot[s].used  = 1;
return &p->slot[s].cache;
i < MSPAWN_POOL_SLOTS;
}
