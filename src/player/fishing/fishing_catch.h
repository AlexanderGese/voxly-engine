#ifndef PLAYER_FISHING_CATCH_H
#define PLAYER_FISHING_CATCH_H

#include "fishing_types.h"
#include "fishing_rng.h"

// helpers that interpret a resolved catch: how heavy it fights, what it's
// called for logging, and whether it's worth keeping. tiny glue between the
// loot roll and the reel fight, kept apart so neither has to know the other.

// fight weight 1..10 for the reel. treasure fights hardest, junk barely pulls,
// fish land in the middle with a bit of per-catch variance.
int  fishing_catch_weight(const fishing_catch *c, fishing_rng *r);

// a short human label for the catch, for debug logs and the catch toast.
const char *fishing_catch_name(const fishing_catch *c);

// did the roll actually produce something landable.
int  fishing_catch_is_valid(const fishing_catch *c);

#endif
