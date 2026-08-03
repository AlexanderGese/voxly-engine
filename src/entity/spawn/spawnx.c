#include "spawnx.h"
#include <string.h>

void spawnx_init(spawnx *sx, unsigned world_seed) {
    memset(sx, 0, sizeof *sx);
    sx->world_seed = world_seed;
    sx->next_event_id = 1;             // 0 is reserved for "no event"
    spawnx_anchor_init(&sx->anchors);
    spawnx_region_init(&sx->region);
    spawnx_despawn_init(&sx->despawn);
    // driver-level rng, distinct stream from any spawner's own.
    mspawn_rng_seed(&sx->rng, mspawn_seed_mix((uint32_t)world_seed, 0x5A11Eu));
}

int spawnx_add_spawner(spawnx *sx, int wx, int wy, int wz,
                       entity_type type, biome_id biome) {
    // ignore a dup at the same block, grab the first free slot otherwise.
    int free_slot = -1;
    for (int i = 0; i < SPAWNX_MAX_SPAWNERS; i++) {
        spawnx_blockspawner *bs = &sx->spawner[i];
        if (bs->active && bs->wx == wx && bs->wy == wy && bs->wz == wz)
            return 1;               // already tracked
        if (!bs->active && free_slot < 0) free_slot = i;
    }
    if (free_slot < 0) return 0;
    spawnx_bs_make(&sx->spawner[free_slot], wx, wy, wz, type, biome,
                   sx->world_seed);
    return 1;
}

void spawnx_remove_spawner(spawnx *sx, int wx, int wy, int wz) {
    for (int i = 0; i < SPAWNX_MAX_SPAWNERS; i++) {
        spawnx_blockspawner *bs = &sx->spawner[i];
        if (bs->active && bs->wx == wx && bs->wy == wy && bs->wz == wz) {
            bs->active = 0;
            return;
        }
    }
}

uint32_t spawnx_start_event(spawnx *sx, vec3 focus, biome_id biome,
                            int waves, int wave_budget, float wave_gap) {
    for (int i = 0; i < SPAWNX_MAX_EVENTS; i++) {
        if (spawnx_event_finished(&sx->event[i])) {
            uint32_t id = sx->next_event_id++;
            if (sx->next_event_id == 0) sx->next_event_id = 1;  // skip the 0
            spawnx_event_begin(&sx->event[i], focus, biome, waves, wave_budget,
                               wave_gap, id, sx->world_seed);
            return id;
        }
    }
    return 0;   // all event slots busy
}

int spawnx_persist_mob(spawnx *sx, uint32_t entity_id, int sticky) {
    return spawnx_anchor_add(&sx->anchors, entity_id, SPAWNX_SRC_COMMAND,
                             sticky);
}

void spawnx_tick(spawnx *sx, mob_registry *mr, world *w, vec3 player_pos,
                 float dt) {
    sx->passes++;
    sx->last_block_spawned = 0;
    sx->last_event_spawned = 0;
    sx->last_culled = 0;

    // 1. fresh regional tallies and anchor hygiene before anyone adds mobs.
    spawnx_region_rebuild(&sx->region, mr);
    spawnx_anchor_sweep(&sx->anchors, mr);

    // 2. block spawners. each is self-gated by player range, so iterating all
    // of them every frame is fine; idle ones bail in a couple comparisons.
    for (int i = 0; i < SPAWNX_MAX_SPAWNERS; i++) {
        if (!sx->spawner[i].active) continue;
        sx->last_block_spawned += spawnx_bs_tick(&sx->spawner[i], w, mr,
                                                 &sx->region, player_pos, dt);
    }

    // 3. scripted events. finished ones are no-ops; we leave them parked so
    // their slot frees on the next start_event scan.
    for (int i = 0; i < SPAWNX_MAX_EVENTS; i++) {
        if (spawnx_event_finished(&sx->event[i])) continue;
        sx->last_event_spawned += spawnx_event_tick(&sx->event[i], w, mr,
                                                    &sx->region, dt);
    }

    // 4. anchor-aware despawn. runs every frame; mobs we placed and tagged
    // persistent survive it, ambient drifters thin out as the player leaves.
    sx->last_culled = spawnx_despawn_tick(&sx->despawn, mr, &sx->anchors,
                                          player_pos, &sx->rng, dt);
}
