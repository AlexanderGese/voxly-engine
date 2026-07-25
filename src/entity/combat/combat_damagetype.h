#ifndef ENTITY_COMBAT_COMBAT_DAMAGETYPE_H
#define ENTITY_COMBAT_COMBAT_DAMAGETYPE_H

#include "combat_types.h"
#include <stdbool.h>

// per-damage-type metadata. one row per combat_damage_type.

typedef struct {
    const char *name;
    bool  ignores_armor;    // void / magic skip the armor calc
    bool  ignores_iframes;  // fire / void tick past the invuln window
    bool  does_knockback;   // fall / drown shouldnt shove you
    float knockback_mult;   // scales the hits base knockback
    int   default_amount;   // sensible damage if a caller passes 0
} combat_dmg_info;

// fetch the row for a type. always returns a valid pointer (clamps oob to
// GENERIC) so callers never have to null-check.
const combat_dmg_info *combat_dmg_info_get(combat_damage_type t);

// short name for logs / debug ui.
const char *combat_dmg_name(combat_damage_type t);

// does this type get to ignore the target armor?
bool combat_dmg_ignores_armor(combat_damage_type t);

// does this type tick through invulnerability frames?
bool combat_dmg_ignores_iframes(combat_damage_type t);

#endif
