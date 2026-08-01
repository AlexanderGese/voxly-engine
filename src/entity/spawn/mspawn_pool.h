#ifndef ENTITY_SPAWN_MSPAWN_POOL_H
#define ENTITY_SPAWN_MSPAWN_POOL_H
#include "mspawn_local.h"
#include "../../world/world.h"
// a tiny fixed ring of per-chunk spot caches. the driver works within a window
// around the player, which is a handful of chunks, so we dont need a real
// hashmap here; an N-slot ring with linear scan is faster than hashing for N
// this small and never allocates. on a miss we evict the oldest slot and
#define MSPAWN_POOL_SLOTS 8
typedef struct {
    mspawn_local cache;
    uint64_t     stamp;     // logical clock of last (re)build, for LRU
    int         used;
} mspawn_pool_slot;
typedef struct {
    mspawn_pool_slot slot[MSPAWN_POOL_SLOTS];
    uint64_t         clock;     // ticks up on every access
    int             hits, misses;   // cheap instrumentation
} mspawn_pool;
void mspawn_pool_init(mspawn_pool *p);
const mspawn_local *mspawn_pool_get(mspawn_pool *p, world *w, int cx, int cz);
void mspawn_pool_invalidate(mspawn_pool *p, int cx, int cz);
void mspawn_pool_clear(mspawn_pool *p);
#endif
