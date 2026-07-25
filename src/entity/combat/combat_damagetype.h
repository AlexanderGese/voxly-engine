#ifndef ENTITY_COMBAT_COMBAT_DAMAGETYPE_H
#define ENTITY_COMBAT_COMBAT_DAMAGETYPE_H
#include "combat_types.h"
#include <stdbool.h>
typedef struct {
    const char *name;
    bool  ignores_armor;    // void / magic skip the armor calc
    bool  ignores_iframes;  // fire / void tick past the invuln window
    bool  does_knockback;   // fall / drown shouldnt shove you
    float knockback_mult;   // scales the hits base knockback
    int   default_amount;   // sensible damage if a caller passes 0
} combat_dmg_info;
#endif
