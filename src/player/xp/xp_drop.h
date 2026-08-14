#ifndef PLAYER_XP_XP_DROP_H
#define PLAYER_XP_XP_DROP_H
// turns an abstract "this gave N xp" event into actual orbs in the pool.
// the source enum lets us vary the spread/value-per-event without callers
// caring about tier math.
#include "../../math/vec3.h"
typedef enum {
    XP_SRC_MOB,      // killed something
    XP_SRC_ORE,      // mined ore
    XP_SRC_SMELT,    // pulled from a furnace
    XP_SRC_BREED,    // animal breeding
    XP_SRC_BOTTLE,   // thrown xp bottle, instant-ish
    XP_SRC_DEBUG,    // command
} xp_source;
typedef struct xp_orb_pool xp_orb_pool;
// suggested xp value for common mob types. centralizes the "how much is a
int  xp_drop_value_for_mob(int entity_type);
int  xp_drop_spawn(xp_orb_pool *pool, vec3 pos, int amount, xp_source src);
#endif
