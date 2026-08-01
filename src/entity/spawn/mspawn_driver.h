#ifndef ENTITY_SPAWN_MSPAWN_DRIVER_H
#define ENTITY_SPAWN_MSPAWN_DRIVER_H

#include "mspawn_types.h"
#include "mspawn_rand.h"
#include "mspawn_density.h"
#include "mspawn_despawn.h"
#include "mspawn_budget.h"
#include "../mob.h"
#include "../../world/world.h"

// the spawn driver. owns the per-frame loop that the rest of the modules feed:
// rebuild density -> despawn -> attempt spawns. one of these per world. this is
// the replacement for the old entity/spawner.c; it keeps the mspawn_ prefix so
// both can live side by side until the old one is deleted.

// how often a spawn attempt cycle fires, seconds. despawn runs every tick.
#define MSPAWN_INTERVAL    2.0f

// darts thrown per cycle, per open category. each dart is a packs worth.
#define MSPAWN_ATTEMPTS    6

// ring the player we spawn in, in blocks. inner avoids spawning on top of them.
#define MSPAWN_RING_INNER  24
#define MSPAWN_RING_OUTER  64

typedef struct {
    mspawn_rng           rng;
    mspawn_density       density;
    mspawn_despawn_state despawn;
    mspawn_budget_cfg    budget;      // difficulty / moon knobs
    float                timer;       // accumulates toward MSPAWN_INTERVAL
    int                  last_hostile_cap;  // resolved cap, for debug overlay
    // rolling counters, handy for a debug overlay.
    int                  last_spawned;
    int                  last_culled;
    uint64_t             passes;
} mspawn_driver;

// seed off the world seed so a save replays the same spawn stream-ish.
void mspawn_driver_init(mspawn_driver *dr, unsigned world_seed);

// push the current difficulty + moon phase. cheap, call it whenever those
// change (settings menu, day rollover); the next cycle picks it up.
void mspawn_driver_set_difficulty(mspawn_driver *dr, mspawn_difficulty diff);
void mspawn_driver_set_moon(mspawn_driver *dr, int moon_phase);

// call every frame. handles its own interval gating internally, so just feed
// it dt and the current clock. mutates the registry (spawn + despawn).
void mspawn_driver_tick(mspawn_driver *dr, mob_registry *mr, world *w,
                        vec3 player_pos, float day_hour, float dt);

#endif
