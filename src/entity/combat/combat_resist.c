#include "combat_resist.h"
#include "combat_config.h"
#include "combat_damagetype.h"
#define RESIST_GRANT_MULT    0.5f
#define RESIST_WEAK_MULT     1.5f
;
i < COMBAT_DMG_COUNT;
i++) c->resist[i] = 1.0f;
if (mult < 0.0f) mult = 0.0f;
c->resist[t] *= mult;
if (c->resist[t] > 8.0f) c->resist[t] = 8.0f;
