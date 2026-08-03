#ifndef ENTITY_SPAWN_SPAWNX_H
#define ENTITY_SPAWN_SPAWNX_H

#include "spawnx_types.h"
#include "spawnx_anchor.h"
#include "spawnx_region.h"
#include "spawnx_blockspawner.h"
#include "spawnx_event.h"
#include "spawnx_persist.h"
#include "mspawn_rand.h"
#include "../mob.h"
#include "../../world/world.h"

// the deliberate-spawning system as one object. owns the block spawners, the
// active scripted events, the persistence anchors, the regional cap map, and
// the anchor-aware despawn pass. the ambient mspawn driver runs alongside this;
// the two only ever touch the same mob_registry and otherwise stay out of each
// other's way (mspawn does ambient fill, spawnx does placed/scripted/persist).
//
// one of these per world. tick it every frame after the ambient driver.

#define SPAWNX_MAX_SPAWNERS 32   // block-spawner cages tracked at once
#define SPAWNX_MAX_EVENTS    4   // concurrent scripted events

typedef struct {
    spawnx_blockspawner  spawner[SPAWNX_MAX_SPAWNERS];
    spawnx_event         event[SPAWNX_MAX_EVENTS];
    spawnx_anchor_store  anchors;
    spawnx_region_map    region;
    spawnx_despawn_state despawn;
    mspawn_rng           rng;
    unsigned             world_seed;
    uint32_t             next_event_id;   // monotonic, never reuses 0

    // rolling counters for a debug overlay.
    int last_block_spawned;
    int last_event_spawned;
    int last_culled;
    uint64_t passes;
} spawnx;

void spawnx_init(spawnx *sx, unsigned world_seed);

// register a monster-spawner cage discovered in the world (dungeon gen calls
// this). returns 1, or 0 if the spawner table is full. dups are ignored.
int  spawnx_add_spawner(spawnx *sx, int wx, int wy, int wz,
                        entity_type type, biome_id biome);

// drop a spawner cage (block broken / chunk unloaded). safe if absent.
void spawnx_remove_spawner(spawnx *sx, int wx, int wy, int wz);

// kick off a scripted event around a focus point. returns its handle, or 0 if
// no event slot is free.
uint32_t spawnx_start_event(spawnx *sx, vec3 focus, biome_id biome,
                            int waves, int wave_budget, float wave_gap);

// mark a live mob persistent (named/tamed/command). thin pass-through to the
// anchor store using the system's own ids.
int  spawnx_persist_mob(spawnx *sx, uint32_t entity_id, int sticky);

// the per-frame entry point. rebuilds the region map, sweeps dead anchors,
// ticks spawners and events, then runs the anchor-aware despawn. mutates the
// registry. cheap enough to call unconditionally every frame.
void spawnx_tick(spawnx *sx, mob_registry *mr, world *w, vec3 player_pos,
                 float dt);

#endif
