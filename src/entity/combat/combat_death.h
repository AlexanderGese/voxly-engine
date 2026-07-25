#ifndef ENTITY_COMBAT_COMBAT_DEATH_H
#define ENTITY_COMBAT_COMBAT_DEATH_H

#include "combat_types.h"
#include <stdint.h>
#include <stdbool.h>

// death handling. when a combatant hits 0 hp the resolver just flips its
// dead flag; the actual consequences (drops, despawn, kill credit) get
// queued here and drained by the game loop so combat stays side-effect free.

typedef struct {
    uint32_t           victim_id;
    uint32_t           killer_id;     // 0 = environment / suicide
    combat_damage_type cause;
    vec3               pos;           // where it died, for loot spawn
    float              despawn_timer; // counts up; body lingers then vanishes
    bool               drops_done;    // loot already spawned?
} combat_death_event;

// fixed-size ring of pending deaths. plenty for a chunk full of mobs.
#define COMBAT_DEATH_QUEUE_MAX 64

typedef struct {
    combat_death_event events[COMBAT_DEATH_QUEUE_MAX];
    int count;
} combat_death_queue;

void combat_death_queue_init(combat_death_queue *q);

// record a death. dedupes on victim_id so a double-resolve doesnt double
// the loot. returns the event slot, or NULL if the queue is full.
combat_death_event *combat_death_push(combat_death_queue *q,
                                      const combat_combatant *victim, vec3 pos);

// find the pending death for a victim, or NULL.
combat_death_event *combat_death_find(combat_death_queue *q, uint32_t victim_id);

// advance despawn timers. removes events whose body has lingered past
// `linger` seconds (and whose drops are done). returns how many were reaped.
int combat_death_tick(combat_death_queue *q, float dt, float linger);

// pop the oldest event that still needs its drops spawned, mark it handled.
// returns NULL when there is nothing left to drop. drain this in a loop.
combat_death_event *combat_death_next_drop(combat_death_queue *q);

#endif
