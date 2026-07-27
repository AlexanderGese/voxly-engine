#ifndef ENTITY_ECS_SYS_COMBAT_H
#define ENTITY_ECS_SYS_COMBAT_H

#include "ecs_world.h"
#include "ecs_events.h"

// the damage resolution pass. it's the consumer side of the event bus: other
// code (the player's swing, a projectile hit, fall damage from the collide
// pass) emits ECS_EV_DAMAGED, and this system drains those each frame, applies
// them to ecs_health respecting i-frames, and emits ECS_EV_DIED for anything it
// kills. keeping damage application in one place means i-frame and death rules
// live in exactly one spot instead of smeared across every attacker.
//
// it deliberately does *not* destroy the dead entity -- that's the health
// system's job (or loot's, off the DIED event). this pass only mutates hp and
// announces deaths, so the same frame's loot/xp listeners get a clean signal
// before the entity gets reaped.

typedef struct {
    ecs_events *events;       // shared bus; read DAMAGED, write DIED
    float       iframe_time;  // seconds of invulnerability after a hit lands
    float       knockback;    // horizontal impulse applied on hit, blocks/sec
} ecs_combat_ctx;

void ecs_combat_ctx_defaults(ecs_combat_ctx *c, ecs_events *events);

// queue a hit. convenience over hand-building the event; clamps amount >= 0.
void ecs_deal_damage(ecs_combat_ctx *c, ecs_entity src, ecs_entity dst,
                     float amount);

// register after ai/movement. drains the DAMAGED events the bus swapped in this
// frame and applies them. user is an ecs_combat_ctx*.
void ecs_sys_combat(ecs_world *w, float dt, void *user);

#endif
