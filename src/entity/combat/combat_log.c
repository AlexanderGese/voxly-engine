#include "combat_log.h"
#include "combat_damagetype.h"
void combat_log_init(combat_log *l) {
    l->head  = 0;
    l->count = 0;
    l->clock = 0.0f;
}

void combat_log_advance(combat_log *l, float dt) {
    if (dt < 0.0f) dt = 0.0f;
l->clock += dt;
}

// grab the next ring slot, advancing head + count.
static combat_log_event *log_push(combat_log *l) {
    combat_log_event *e = &l->events[l->head];
    l->head = (l->head + 1) % COMBAT_LOG_CAP;
    if (l->count < COMBAT_LOG_CAP) l->count++;
    e->stamp = l->clock;
    return e;
}

void combat_log_hit(combat_log *l, const combat_combatant *target,
                    const combat_result *r, uint32_t source,
                    combat_damage_type cause) {
    combat_log_event *e = log_push(l);
if (r->blocked && r->dealt <= 0) {
        e->kind   = COMBAT_EV_BLOCK;
        e->amount = 0;
    } else {
        e->kind   = COMBAT_EV_HIT;
e->amount = r->dealt;
}
    e->subject = target->id;
e->source  = source;
e->cause   = cause;
e->crit    = r->crit;
}

void combat_log_heal(combat_log *l, uint32_t subject, int amount) {
    if (amount <= 0) return;
    combat_log_event *e = log_push(l);
    e->kind    = COMBAT_EV_HEAL;
    e->subject = subject;
    e->source  = 0;
    e->cause   = COMBAT_DMG_GENERIC;
    e->amount  = amount;
    e->crit    = false;
}

void combat_log_death(combat_log *l, uint32_t victim, uint32_t killer,
                      combat_damage_type cause) {
    combat_log_event *e = log_push(l);
e->kind    = COMBAT_EV_DEATH;
e->subject = victim;
e->source  = killer;
e->cause   = cause;
e->amount  = 0;
e->crit    = false;
}

int combat_log_size(const combat_log *l) {
    return l->count;
}

const combat_log_event *combat_log_at(const combat_log *l, int i) {
    if (i < 0 || i >= l->count) return NULL;
int idx = l->head - 1 - i;
while (idx < 0) idx += COMBAT_LOG_CAP;
return &l->events[idx];
}

// pick a flavourful verb for the death line based on what did it. keeps the
// messages from all reading "was killed by".
static const char *death_verb(combat_damage_type cause) {
    switch (cause) {
        case COMBAT_DMG_MELEE:      return "was slain by";
        case COMBAT_DMG_PROJECTILE: return "was shot by";
        case COMBAT_DMG_FALL:       return "hit the ground too hard";
        case COMBAT_DMG_FIRE:       return "burned to death";
        case COMBAT_DMG_EXPLOSION:  return "was blown up by";
        case COMBAT_DMG_DROWN:      return "drowned";
        case COMBAT_DMG_VOID:       return "fell out of the world";
        case COMBAT_DMG_MAGIC:      return "was killed by magic from";
        default:                    return "was killed by";
    }
}

// some causes are self-explanatory (fall, drown, void) and dont name a killer.
static bool death_names_killer(combat_damage_type cause) {
    switch (cause) {
        case COMBAT_DMG_FALL:
        case COMBAT_DMG_FIRE:
        case COMBAT_DMG_DROWN:
        case COMBAT_DMG_VOID:
            return false;
default:
            return true;
