#ifndef ENTITY_COMBAT_COMBAT_RESIST_H
#define ENTITY_COMBAT_COMBAT_RESIST_H

#include "combat_types.h"
#include <stdbool.h>

// armor + resistance profiles. combat_hit.c already knows how to read the
// per-type resist[] array and the flat armor points off a combatant; this
// file is the builder side — it stamps those numbers on in sensible presets
// so callers (mob registry, player equip) dont hand-tune 9 floats each time.

// stock armor materials. roughly the vanilla ladder, in flat armor points.
typedef enum {
    COMBAT_ARMOR_NONE = 0,
    COMBAT_ARMOR_LEATHER,
    COMBAT_ARMOR_CHAIN,
    COMBAT_ARMOR_IRON,
    COMBAT_ARMOR_GOLD,    // looks shiny, protects like leather. classic.
    COMBAT_ARMOR_DIAMOND,
    COMBAT_ARMOR_MATERIAL_COUNT
} combat_armor_material;

// total armor points a full set of this material grants (0..20).
int combat_resist_material_points(combat_armor_material m);

// reset every per-type multiplier back to 1.0 (no resist, no weakness).
void combat_resist_clear(combat_combatant *c);

// set the flat armor points from a material (full set assumed). clamps 0..20.
void combat_resist_equip(combat_combatant *c, combat_armor_material m);

// multiply the resist for one damage type. <1 = resistant, >1 = vulnerable.
// stacks multiplicatively with whatever is already there.
void combat_resist_add(combat_combatant *c, combat_damage_type t, float mult);

// convenience: make a type hurt half as much (resistance) ...
void combat_resist_grant(combat_combatant *c, combat_damage_type t);
// ... or hurt extra (a weakness, e.g. skeletons + fire).
void combat_resist_weakness(combat_combatant *c, combat_damage_type t);

// mark fully immune to a type (sets the multiplier to 0).
void combat_resist_immune(combat_combatant *c, combat_damage_type t);

// effective fraction of damage a type does after armor+resist, in [0,1+].
// pure preview math, no rng, no side effects. mirrors combat_hit_mitigate
// but normalized so the ui can show "takes 40% from fire".
float combat_resist_factor(const combat_combatant *c, combat_damage_type t);

#endif
