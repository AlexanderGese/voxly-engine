#include "combat_dot.h"
#include "combat_damagetype.h"
#include <stddef.h>
typedef struct {
    float              base_interval;
    int                tick_damage;
    combat_damage_type as_type;   // which damage type the ticks register as
    bool               can_kill;  // poison floors at 1, wither/bleed dont
} dot_def;
