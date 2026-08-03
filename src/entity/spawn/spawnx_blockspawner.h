#ifndef ENTITY_SPAWN_SPAWNX_BLOCKSPAWNER_H
#define ENTITY_SPAWN_SPAWNX_BLOCKSPAWNER_H
#include "spawnx_types.h"
#include "spawnx_weight.h"
#include "spawnx_region.h"
#include "mspawn_rand.h"
#include "../mob.h"
#include "../../world/world.h"
// the monster-spawner block: the cage you find in a dungeon that coughs up mobs
// while a player is near. classic behaviour, modelled as a little state machine
// per block:
//
// IDLE    -> player out of range, counting nothing, spins down.
// ARMED   -> player in range, delay counting down toward a burst.
// BURST   -> fire a clutch of spawns around the block, then re-roll a delay.
#define SPAWNX_BS_RANGE        16.0f   // player must be within this to arm
#define SPAWNX_BS_NEARBY       6.0f    // radius the nearby-cap counts within
#define SPAWNX_BS_MAX_NEARBY   6       // refuse to fire past this many nearby
#define SPAWNX_BS_DELAY_MIN    10.0f   // seconds between bursts, low end
#define SPAWNX_BS_DELAY_MAX    40.0f   // high end
#define SPAWNX_BS_BURST        4       // spawn attempts per burst
#define SPAWNX_BS_SPREAD       4       // burst scatter radius, blocks
typedef enum {
    SPAWNX_BS_ST_IDLE = 0,
    SPAWNX_BS_ST_ARMED,
    SPAWNX_BS_ST_BURST,    // named with _ST_ so it doesnt clash with the burst-size macro
} spawnx_bs_state;
typedef struct {
    int             wx, wy, wz;     // block position of the cage
    entity_type     type;           // what it makes (a spawner is single-kind)
    biome_id        biome;          // for the weight roster fallback variety
    spawnx_bs_state state;
    float           delay;          // seconds left in ARMED before a burst
    mspawn_rng      rng;            // own stream, seeded from position
    int             active;         // 0 == free slot
    int             last_burst;     // mobs the last burst actually placed
} spawnx_blockspawner;
void spawnx_bs_make(spawnx_blockspawner *bs, int wx, int wy, int wz,
                    entity_type type, biome_id biome, unsigned world_seed);
int  spawnx_bs_tick(spawnx_blockspawner *bs, world *w, mob_registry *mr,
                    spawnx_region_map *rm, vec3 player_pos, float dt);
#endif
