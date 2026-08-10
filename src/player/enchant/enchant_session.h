#ifndef PLAYER_ENCHANT_SESSION_H
#define PLAYER_ENCHANT_SESSION_H
#include "enchant_types.h"
#include "enchant_table.h"
#include "../../world/world.h"
typedef struct {
    int xp_level;     // whole xp levels available
    int xp_points;    // progress into the current level (for sub-level spend)
    int dye;          // lapis stand-in
} enchant_wallet;
typedef enum {
    ENCHANT_TAKE_OK = 0,
    ENCHANT_TAKE_NO_ITEM,
    ENCHANT_TAKE_BAD_SLOT,
    ENCHANT_TAKE_EMPTY_SLOT,
    ENCHANT_TAKE_POOR_LEVELS,
    ENCHANT_TAKE_POOR_DYE,
} enchant_take_status;
typedef struct {
    enchant_table table;
    int           tx, ty, tz;     // table block position
    int           open;
    enchant_set  *target;         // borrowed: the item's enchant set to write
} enchant_session;
#endif
