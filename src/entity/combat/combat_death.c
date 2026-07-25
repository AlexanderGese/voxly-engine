#include "combat_death.h"

#include <stddef.h>

void combat_death_queue_init(combat_death_queue *q) {
    q->count = 0;
}

combat_death_event *combat_death_find(combat_death_queue *q, uint32_t victim_id) {
    for (int i = 0; i < q->count; i++) {
        if (q->events[i].victim_id == victim_id) return &q->events[i];
    }
    return NULL;
}

combat_death_event *combat_death_push(combat_death_queue *q,
                                      const combat_combatant *victim, vec3 pos) {
    // already queued? dont double up the loot.
    combat_death_event *exist = combat_death_find(q, victim->id);
    if (exist) return exist;

    if (q->count >= COMBAT_DEATH_QUEUE_MAX) {
        // queue full. drop the request rather than stomp something live.
        // shouldnt happen unless a whole herd dies on one frame.
        return NULL;
    }

    combat_death_event *ev = &q->events[q->count++];
    ev->victim_id     = victim->id;
    ev->killer_id     = victim->last_attacker;
    ev->cause         = victim->last_dmg_type;
    ev->pos           = pos;
    ev->despawn_timer = 0.0f;
    ev->drops_done    = false;
    return ev;
}

// swap-remove keeps the array packed. order doesnt matter to consumers.
static void death_remove(combat_death_queue *q, int idx) {
    q->count--;
    if (idx != q->count) q->events[idx] = q->events[q->count];
}

int combat_death_tick(combat_death_queue *q, float dt, float linger) {
    if (dt < 0.0f) dt = 0.0f;
    int reaped = 0;
    for (int i = 0; i < q->count; ) {
        combat_death_event *ev = &q->events[i];
        ev->despawn_timer += dt;
        // only reap once the body has lingered AND loot has been spawned,
        // otherwise we could vanish a corpse before dropping its stuff.
        if (ev->despawn_timer >= linger && ev->drops_done) {
            death_remove(q, i);
            reaped++;
            // dont advance i, a swapped-in event now sits here.
        } else {
            i++;
        }
    }
    return reaped;
}

combat_death_event *combat_death_next_drop(combat_death_queue *q) {
    int best = -1;
    float oldest = -1.0f;
    for (int i = 0; i < q->count; i++) {
        if (q->events[i].drops_done) continue;
        if (q->events[i].despawn_timer > oldest) {
            oldest = q->events[i].despawn_timer;
            best = i;
        }
    }
    if (best < 0) return NULL;
    q->events[best].drops_done = true;
    return &q->events[best];
}
