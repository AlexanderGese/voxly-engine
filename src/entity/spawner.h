#ifndef ENTITY_SPAWNER_H
#define ENTITY_SPAWNER_H

#include "mob.h"
#include "../world/world.h"
#include "../math/vec3.h"

// controlled spawning: manages spawn caps, biome-aware mob selection,
// pack spawning, and despawn by distance.

#define SPAWN_CAP_HOSTILE  30
#define SPAWN_CAP_PASSIVE  15
#define DESPAWN_DISTANCE   80.0f
#define PACK_SIZE_MIN       2
#define PACK_SIZE_MAX       4

typedef struct {
    float timer;
    float interval;
    int   hostile_count;
    int   passive_count;
} spawner;

void spawner_init(spawner *sp);
void spawner_tick(spawner *sp, mob_registry *mr, world *w,
                  vec3 player_pos, float day_hour, float dt);
void spawner_despawn_far(spawner *sp, mob_registry *mr, vec3 player_pos);

#endif
