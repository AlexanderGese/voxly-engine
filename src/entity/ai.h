#ifndef ENTITY_AI_H
#define ENTITY_AI_H

#include "entity.h"
#include "../world/world.h"
#include "../player/player.h"

// state machine: idle -> wander -> chase -> attack -> hurt.
// cheap heuristic "ai" only; no real pathfinding.

typedef enum {
    AI_IDLE = 0,
    AI_WANDER,
    AI_CHASE,
    AI_ATTACK,
    AI_HURT,
    AI_FLEE,
} ai_state;

void ai_tick(entity *e, const player *p, world *w, float dt);

#endif
