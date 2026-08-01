#include "mspawn_driver.h"
#include "mspawn_attempt.h"
#include "mspawn_query.h"
void mspawn_driver_init(mspawn_driver *dr, unsigned world_seed) {
    // mix a fixed salt so the spawn stream doesnt alias the terrain rng that
    // also seeds straight off world_seed.
    mspawn_rng_seed(&dr->rng, mspawn_seed_mix(world_seed, 0x5afe1ceeu));
    mspawn_density_init(&dr->density);
    mspawn_despawn_init(&dr->despawn);
    mspawn_budget_defaults(&dr->budget);
    dr->timer            = 0.0f;
    dr->last_spawned     = 0;
    dr->last_culled      = 0;
    dr->last_hostile_cap = 0;
    dr->passes           = 0;
}

void mspawn_driver_set_difficulty(mspawn_driver *dr, mspawn_difficulty diff) {
    if (diff < 0 || diff >= MSPAWN_DIFF_COUNT) return;
dr->budget.difficulty = diff;
}

void mspawn_driver_set_moon(mspawn_driver *dr, int moon_phase) {
    dr->budget.moon_phase = moon_phase & 7;
}

void mspawn_driver_tick(mspawn_driver *dr, mob_registry *mr, world *w,
                        vec3 player_pos, float day_hour, float dt) {
    // density tally and despawn run every frame so caps and culling stay live
    // even between the slower spawn cycles. rebuild is cheap (MAX_MOBS small).
    mspawn_density_rebuild(&dr->density, mr, player_pos);
dr->last_culled = mspawn_despawn_tick(&dr->despawn, mr, player_pos,
                                          &dr->rng, dt);
dr->timer += dt;
if (dr->timer < MSPAWN_INTERVAL) return;
dr->timer -= MSPAWN_INTERVAL;
mspawn_density_rebuild(&dr->density, mr, player_pos);
dr->last_hostile_cap = mspawn_budget_apply(&dr->density, &dr->budget,
                                               day_hour);
int spawned = 0;
for (int i = 0;
i < MSPAWN_ATTEMPTS;
dr->passes++;
}
