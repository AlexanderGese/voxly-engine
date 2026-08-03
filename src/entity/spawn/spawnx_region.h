#ifndef ENTITY_SPAWN_SPAWNX_REGION_H
#define ENTITY_SPAWN_SPAWNX_REGION_H
#include "spawnx_types.h"
#include "../mob.h"
// regional population accounting for the placed paths. mspawn_density already
// tracks crowding in a window glued to the player; that window slides, so it's
// the wrong tool for "this dungeon region is already at its monster cap whether
// or not the player is standing in it". this is a small fixed open-addressed
// table keyed by region (SPAWNX_REGION blocks square) that the block-spawner and
// event code consult before they add to a region.
//
// fixed-size on purpose: at our mob counts there are only ever a handful of
// active regions, and a flat table that never allocates is easier to reason
// about than dragging the util hashmap in here.
#define SPAWNX_REGION_SLOTS 32   // distinct regions tracked at once
#define SPAWNX_REGION_CAP    6   // mobs per region before we stop adding
typedef struct {
    uint64_t key;     // packed region key, 0 == empty slot
    int      count;   // live mobs whose feet fall in this region
} spawnx_region_slot;
typedef struct {
    spawnx_region_slot slot[SPAWNX_REGION_SLOTS];
    int                used;     // occupied slots, for the eviction heuristic
} spawnx_region_map;
void spawnx_region_init(spawnx_region_map *m);
// recount from the live registry. cheap (MAX_MOBS small); call once per cycle
// before the placed paths run so their checks see this tick's reality.
void spawnx_region_rebuild(spawnx_region_map *m, const mob_registry *mr);
// current live count in the region containing `pos`. 0 if untracked.
int  spawnx_region_count(const spawnx_region_map *m, vec3 pos);
int  spawnx_region_has_room(const spawnx_region_map *m, vec3 pos);
void spawnx_region_account(spawnx_region_map *m, vec3 pos);
#endif
