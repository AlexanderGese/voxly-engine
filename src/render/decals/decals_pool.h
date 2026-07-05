#ifndef RENDER_DECALS_POOL_H
#define RENDER_DECALS_POOL_H
#include "decals_types.h"
#include "decals_config.h"
#include "decals_atlas.h"
// fixed-capacity slot pool for live decals. flat array, free-list of recycled
// indices, generation counters for stale-handle detection. no allocation at
// runtime — DECALS_MAX is the ceiling and we evict the oldest decal when full
// rather than refusing the spawn (a missing footprint is worse than a popped
// one).
// spawn descriptor. the caller fills what it cares about; defaults() seeds the
// rest. region is an atlas index resolved by the system layer before it gets
// here.
typedef struct {
    decals_projector proj;
    decals_atlas_region region; // resolved uv rects (system layer fills these)
    int      has_region;        // 1 once region is resolved; spawn refuses if 0
    float    tint[3];
    float    normal_strength; // 0..1
    float    angle_fade;      // cos cutoff
    float    life_total;      // <0 = permanent
    float    fade_in, fade_out;
    uint16_t flags;
    uint16_t priority;
} decals_spawn_desc;
typedef struct {
    decals_decal slots[DECALS_MAX];
    uint32_t     free_list[DECALS_MAX];  // stack of free indices
    int          free_top;               // count of free indices on the stack
    int          live_count;             // currently-alive decals
    uint32_t     gen_seed;               // bumped per spawn for fresh gens
} decals_pool;
void decals_pool_init(decals_pool *p);
// fill a descriptor with sane defaults. caller overrides what it wants.
void decals_spawn_desc_defaults(decals_spawn_desc *d);
// spawn a decal, returning a handle. evicts the dimmest decal if the pool is
// full. handle is DECALS_INVALID_HANDLE only if the descriptor is degenerate.
decals_handle decals_pool_spawn(decals_pool *p, const decals_spawn_desc *d);
// resolve a handle to a live decal, or NULL if it's dead/recycled/invalid.
decals_decal *decals_pool_get(decals_pool *p, decals_handle h);
// kill a decal early (eases out). no-op on a dead handle.
void decals_pool_kill(decals_pool *p, decals_handle h);
int  decals_pool_tick(decals_pool *p, float dt);
int  decals_pool_live_count(const decals_pool *p);
#endif
