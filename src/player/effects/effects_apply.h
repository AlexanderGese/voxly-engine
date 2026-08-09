#ifndef PLAYER_EFFECTS_EFFECTS_APPLY_H
#define PLAYER_EFFECTS_EFFECTS_APPLY_H
#include "effects_set.h"
#include "effects_curative.h"
typedef enum {
    EFFECTS_APPLY_ADDED = 0,   // landed in a fresh slot
    EFFECTS_APPLY_UPGRADED,    // merged and changed an existing instance
    EFFECTS_APPLY_REFRESHED,   // existing instance's clock pushed out, same level
    EFFECTS_APPLY_IGNORED,     // weaker/shorter than what's running, dropped
    EFFECTS_APPLY_IMMUNE,      // carrier can't receive this kind
    EFFECTS_APPLY_INVALID,     // bad args (kind/duration)
} effects_apply_kind;
#endif
