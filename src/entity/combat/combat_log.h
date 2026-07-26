#ifndef ENTITY_COMBAT_COMBAT_LOG_H
#define ENTITY_COMBAT_COMBAT_LOG_H

#include "combat_types.h"
#include "../../util/strbuf.h"
#include <stdint.h>

// rolling combat log. a fixed ring of recent events (hits, deaths, status)
// the debug overlay scrolls and the death-message line reads from. keeps the
// last N so we dont grow unbounded during a long fight.

typedef enum {
    COMBAT_EV_HIT = 0,    // someone took damage
    COMBAT_EV_BLOCK,      // a hit landed but was fully absorbed
    COMBAT_EV_HEAL,       // regen / potion topped someone up
    COMBAT_EV_DEATH,      // a combatant dropped
    COMBAT_EV_KIND_COUNT
} combat_log_kind;

typedef struct {
    combat_log_kind    kind;
    uint32_t           subject;   // who it happened to
    uint32_t           source;    // who caused it (0 = environment)
    combat_damage_type cause;
    int                amount;    // damage / heal magnitude
    bool               crit;
    float              stamp;     // game time the event was logged at
} combat_log_event;

#define COMBAT_LOG_CAP 32

typedef struct {
    combat_log_event events[COMBAT_LOG_CAP];
    int   head;   // index of the next write (ring)
    int   count;  // how many valid entries (<= cap)
    float clock;  // running clock the log stamps events with
} combat_log;

void combat_log_init(combat_log *l);

// advance the internal clock so events get a sensible timestamp. call once
// per frame before pushing the frame's events.
void combat_log_advance(combat_log *l, float dt);

// record events. these are thin — the caller already has the numbers from a
// combat_result, this just files them.
void combat_log_hit(combat_log *l, const combat_combatant *target,
                    const combat_result *r, uint32_t source,
                    combat_damage_type cause);
void combat_log_heal(combat_log *l, uint32_t subject, int amount);
void combat_log_death(combat_log *l, uint32_t victim, uint32_t killer,
                      combat_damage_type cause);

// number of stored events, and indexed access from newest (0) to oldest.
int combat_log_size(const combat_log *l);
const combat_log_event *combat_log_at(const combat_log *l, int i);

// format the most recent death into a human line ("entity 12 was slain by
// entity 4"). writes into `out`. returns false if there's no death logged.
bool combat_log_death_message(const combat_log *l, strbuf *out);

#endif
