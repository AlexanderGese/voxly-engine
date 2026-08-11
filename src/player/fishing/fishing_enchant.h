#ifndef PLAYER_FISHING_ENCHANT_H
#define PLAYER_FISHING_ENCHANT_H

#include "fishing_types.h"
#include "fishing_rod.h"
#include "../enchant/enchant_types.h"

// glue between the engine's enchant system and our rod stats. the enchant
// registry doesn't (yet) carry lure/luck-of-the-sea ids, so we map the closest
// existing ones: efficiency stands in for lure (faster results) and fortune for
// luck of the sea (better drops). when the real ids land this is the one file
// that has to change.

// pull a lure/luck pair out of an enchant_set and build a rod from it.
fishing_rod fishing_enchant_resolve(const enchant_set *set);

// read just the lure-equivalent level (0..FISHING_LURE_MAX) from a set.
int  fishing_enchant_lure_level(const enchant_set *set);

// read just the luck-equivalent level (0..FISHING_LUCK_MAX) from a set.
int  fishing_enchant_luck_level(const enchant_set *set);

// can this enchant id meaningfully go on a fishing rod. used to filter the
// table offer so you don't get sharpness on your rod.
int  fishing_enchant_applies(enchant_id id);

#endif
