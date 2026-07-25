#ifndef ENTITY_COMBAT_COMBAT_LOG_H
#define ENTITY_COMBAT_COMBAT_LOG_H
#include "combat_types.h"
#include "../../util/strbuf.h"
#include <stdint.h>
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
void combat_log_advance(combat_log *l, float dt);
void combat_log_hit(combat_log *l, const combat_combatant *target,
                    const combat_result *r, uint32_t source,
                    combat_damage_type cause);
void combat_log_heal(combat_log *l, uint32_t subject, int amount);
#endif
