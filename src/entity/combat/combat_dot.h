#ifndef ENTITY_COMBAT_COMBAT_DOT_H
#define ENTITY_COMBAT_COMBAT_DOT_H

#include "combat_types.h"
#include "combat_hit.h"

// damage over time. poison, bleed, wither — effects that sit on a combatant
// and chip at it every interval. the env fire in combat_environment.c is the
// same idea but hardcoded; this is the general, stackable version that ai /
// potions / enchants drive.
//
// these route their ticks through combat_hit_apply with BYPASS_IFRAMES so a
// poisoned mob still flinches from a sword without the poison eating the
// invuln window.

typedef enum {
    COMBAT_DOT_POISON = 0,  // wont kill you, floors at 1 hp
    COMBAT_DOT_BLEED,       // physical, plain damage, can kill
    COMBAT_DOT_WITHER,      // like poison but DOES kill, ignores armor
    COMBAT_DOT_KIND_COUNT
} combat_dot_kind;

// one active effect instance.
typedef struct {
    combat_dot_kind kind;
    int      amplifier;   // 0 = base, each level shortens the interval
    float    remaining;   // seconds of effect left
    float    tick_accum;  // counts toward the next damage tick
    uint32_t source_id;   // who applied it (kill credit)
    bool     active;
} combat_dot;

// up to this many concurrent effects per combatant. refreshing an existing
// kind reuses its slot rather than eating a new one.
#define COMBAT_DOT_MAX 4

typedef struct {
    combat_dot slots[COMBAT_DOT_MAX];
} combat_dot_set;

void combat_dot_init(combat_dot_set *s);

// apply (or refresh) an effect. a stronger amplifier or a longer duration
// upgrades an existing instance of the same kind; weaker ones are ignored.
// returns true if anything changed.
bool combat_dot_apply(combat_dot_set *s, combat_dot_kind kind, int amplifier,
                      float duration, uint32_t source_id);

// is any effect of this kind currently running?
bool combat_dot_has(const combat_dot_set *s, combat_dot_kind kind);

// strip every effect (milk bucket, death, respawn).
void combat_dot_clear(combat_dot_set *s);

// strip just one kind. returns true if one was present.
bool combat_dot_cure(combat_dot_set *s, combat_dot_kind kind);

// advance all effects, applying ticks of damage through the resolver. call
// once per frame. returns total damage dealt this frame across all effects.
int combat_dot_tick(combat_combatant *c, combat_dot_set *s, float dt,
                    combat_rng *rng);

#endif
