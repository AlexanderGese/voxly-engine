#ifndef PLAYER_ENCHANT_SESSION_H
#define PLAYER_ENCHANT_SESSION_H

#include "enchant_types.h"
#include "enchant_table.h"
#include "../../world/world.h"

// the glue between a table, the item being worked, and the player's wallet.
// the ui layer drives this: open a session when the player interacts with a
// table block, feed it the item, ask which slots are affordable, and commit a
// take. keeping this here (not in ui/) means the wallet rules live next to the
// cost rules and the ui stays dumb.

// the player resources a session reads and writes. the host owns the storage;
// the session only ever decrements on a successful take.
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

// begin a session at a table block. snapshots a seed from world+position.
void enchant_session_open(enchant_session *ss, unsigned world_seed,
                          int tx, int ty, int tz);

// attach the item being enchanted and recompute offers. `target` is the item's
// own enchant_set which a successful take will overwrite. pass its category and
// material enchantability. clears the session if item_cat is NONE.
void enchant_session_set_item(enchant_session *ss, world *w, enchant_set *target,
                              enchant_cat item_cat, int enchantability);

// is the given slot currently a live, affordable offer for this wallet?
int  enchant_session_slot_affordable(const enchant_session *ss, int slot,
                                     const enchant_wallet *wallet);

// commit a take: validates, debits the wallet, writes the rolled enchants into
// the attached target item, and voids the other offers. returns a status code.
enchant_take_status enchant_session_take(enchant_session *ss, int slot,
                                         enchant_wallet *wallet);

void enchant_session_close(enchant_session *ss);

#endif
