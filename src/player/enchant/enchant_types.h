#ifndef PLAYER_ENCHANT_TYPES_H
#define PLAYER_ENCHANT_TYPES_H
#include <stdint.h>
#include "../../world/block.h"
// shared vocabulary for the enchanting subsystem. everything else includes
// this. kept dependency-light on purpose: no gl, no world, just ids and the
// little structs that get passed around between the table, the roll picker,
#define ENCHANT_MAX_KINDS      32
#define ENCHANT_MAX_ON_ITEM     8   // how many distinct enchants one item holds
#define ENCHANT_MAX_LEVEL      10   // absolute ceiling for any single enchant
#define ENCHANT_TABLE_SLOTS     3   // the three offered choices, mc-style
#define ENCHANT_MAX_BOOKSHELVES 15  // power saturates here
typedef uint8_t enchant_id;
enum {
    ENCHANT_NONE = 0,
    ENCHANT_SHARPNESS,      // flat melee bonus
    ENCHANT_PROTECTION,     // generic damage soak
    ENCHANT_EFFICIENCY,     // mining speed
    ENCHANT_UNBREAKING,     // durability saver
    ENCHANT_FORTUNE,        // extra drops
    ENCHANT_SILK_TOUCH,     // drop the block itself
    ENCHANT_FIRE_ASPECT,    // set target alight
    ENCHANT_KNOCKBACK,      // shove
    ENCHANT_FEATHER_FALL,   // fall damage soak
    ENCHANT_RESPIRATION,    // breathe underwater longer
    ENCHANT_AQUA_AFFINITY,  // mine fast underwater
    ENCHANT_THORNS,         // reflect damage
    ENCHANT_POWER,          // bow damage
    ENCHANT_PUNCH,          // bow knockback
    ENCHANT_INFINITY,       // arrowless
    ENCHANT_COUNT
}
;
typedef enum {
    ENCHANT_CAT_NONE   = 0,
    ENCHANT_CAT_SWORD  = 1 << 0,
    ENCHANT_CAT_TOOL   = 1 << 1,   // pick/axe/shovel
    ENCHANT_CAT_ARMOR  = 1 << 2,
    ENCHANT_CAT_BOW    = 1 << 3,
    ENCHANT_CAT_BOOK   = 1 << 4,   // books take anything
} enchant_cat;
typedef enum {
    ENCHANT_RARITY_COMMON    = 0,
    ENCHANT_RARITY_UNCOMMON  = 1,
    ENCHANT_RARITY_RARE      = 2,
    ENCHANT_RARITY_LEGENDARY = 3,
    ENCHANT_RARITY_COUNT
} enchant_rarity;
typedef struct {
    enchant_id id;
    uint8_t    level;
} enchant_entry;
typedef struct {
    enchant_entry entry[ENCHANT_MAX_ON_ITEM];
    int           count;
} enchant_set;
typedef struct {
    enchant_id     id;
    const char    *name;
    enchant_cat    cats;        // bitmask of compatible categories
    enchant_rarity rarity;
    uint8_t        max_level;
    uint8_t        weight;      // selection weight, higher = more common
    // anvil cost multiplier per level, in xp levels. the legendary enchants
    // are deliberately painful here.
    uint8_t        anvil_cost;
    // enchants that cannot coexist on the same item (mutually exclusive),
    // terminated by ENCHANT_NONE. e.g. silk touch vs fortune.
    enchant_id     conflicts[4];
} enchant_def;
#endif
