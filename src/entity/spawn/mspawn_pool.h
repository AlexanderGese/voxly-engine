#ifndef ENTITY_SPAWN_MSPAWN_POOL_H
#define ENTITY_SPAWN_MSPAWN_POOL_H

#include "mspawn_local.h"
#include "../../world/world.h"

// a tiny fixed ring of per-chunk spot caches. the driver works within a window
// around the player, which is a handful of chunks, so we dont need a real
// hashmap here; an N-slot ring with linear scan is faster than hashing for N
// this small and never allocates. on a miss we evict the oldest slot and
// rebuild it from the live chunk.
//
// caches go stale when a chunk is edited; rather than wire dirty flags through
// here we just age entries out and rebuild on a generation bump the world hands
// us. coarse but it self-heals within a few cycles.

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

// fetch (or build) the spot cache for chunk (cx, cz). returns NULL only if the
// chunk isnt loaded. the returned pointer is valid until the next eviction, so
// dont hold it across calls.
const mspawn_local *mspawn_pool_get(mspawn_pool *p, world *w, int cx, int cz);

// drop a chunk's cache, e.g. when the world tells us it was edited or unloaded.
void mspawn_pool_invalidate(mspawn_pool *p, int cx, int cz);

// drop everything. cheap, used on world teardown / reload.
void mspawn_pool_clear(mspawn_pool *p);

#endif
