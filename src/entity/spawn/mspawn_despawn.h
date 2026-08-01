#ifndef ENTITY_SPAWN_MSPAWN_DESPAWN_H
#define ENTITY_SPAWN_MSPAWN_DESPAWN_H
#include "mspawn_types.h"
#include "mspawn_rand.h"
#include "../mob.h"
#define MSPAWN_DESPAWN_HARD   80.0f   // beyond this, vanish now
#define MSPAWN_DESPAWN_SOFT   48.0f   // beyond this, start rolling to leave
#define MSPAWN_DESPAWN_CHANCE  0.02f  // per-tick leave chance in the soft band
typedef struct {
    float linger[MAX_MOBS];   // seconds spent in the soft band
} mspawn_despawn_state;
#endif
