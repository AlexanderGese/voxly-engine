#ifndef PLAYER_FISHING_ROD_H
#define PLAYER_FISHING_ROD_H

#include "fishing_types.h"

// rod stats and the small bits of math that depend only on them: how the lure
// enchant trims the wait, how luck shifts the loot weights, and what tension
// the line can take before it snaps. enchant levels clamp to 0..3.

#define FISHING_LURE_MAX   3
#define FISHING_LUCK_MAX   3

// a plain default rod: no enchants, middling line strength.
fishing_rod fishing_rod_default(void);

// build a rod from raw enchant levels. clamps both into range.
fishing_rod fishing_rod_make(int lure, int luck);

// seconds shaved off a bite wait per lure level. flat 5s each, the classic.
float fishing_rod_lure_bonus(const fishing_rod *r);

// extra weight handed to treasure entries per luck level, as a multiplier on
// their base weight. junk gets the inverse so it thins out.
float fishing_rod_treasure_bias(const fishing_rod *r);
float fishing_rod_junk_bias(const fishing_rod *r);

#endif
