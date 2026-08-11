#ifndef PLAYER_ENCHANT_TYPES_H
#define PLAYER_ENCHANT_TYPES_H

#include <stdint.h>
#include "../../world/block.h"

// shared vocabulary for the enchanting subsystem. everything else includes
// this. kept dependency-light on purpose: no gl, no world, just ids and the
// little structs that get passed around between the table, the roll picker,
// and the anvil.

// hard caps. the registry is fixed-size; we never grow it at runtime.
#define ENCHANT_MAX_KINDS      32
#define ENCHANT_MAX_ON_ITEM     8   // how many distinct enchants one item holds
#define ENCHANT_MAX_LEVEL      10   // absolute ceiling for any single enchant
#define ENCHANT_TABLE_SLOTS     3   // the three offered choices, mc-style
#define ENCHANT_MAX_BOOKSHELVES 15  // power saturates here

// enchant ids. plain small integers so they pack into a byte. 0 is "none"
// so a zeroed slot reads as empty without extra bookkeeping.
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
};

// what an item is allowed to carry. an enchant declares a mask of the
// categories it applies to; an item declares its own category. the picker
// only offers compatible enchants.
typedef enum {
    ENCHANT_CAT_NONE   = 0,
    ENCHANT_CAT_SWORD  = 1 << 0,
    ENCHANT_CAT_TOOL   = 1 << 1,   // pick/axe/shovel
    ENCHANT_CAT_ARMOR  = 1 << 2,
    ENCHANT_CAT_BOW    = 1 << 3,
    ENCHANT_CAT_BOOK   = 1 << 4,   // books take anything
} enchant_cat;

// a rarity bucket. drives both how often an enchant shows up in a roll and
// how much it costs to splice in at the anvil. lower weight = rarer.
typedef enum {
    ENCHANT_RARITY_COMMON    = 0,
    ENCHANT_RARITY_UNCOMMON  = 1,
    ENCHANT_RARITY_RARE      = 2,
    ENCHANT_RARITY_LEGENDARY = 3,
    ENCHANT_RARITY_COUNT
} enchant_rarity;

// one (enchant, level) pair living on an item. level 0 means the slot is
// empty regardless of id.
typedef struct {
    enchant_id id;
    uint8_t    level;
} enchant_entry;

// the full set of enchants on a single item. fixed array + count; we keep it
// sorted by id so two equal sets compare cheaply and the anvil merge is
// deterministic.
typedef struct {
    enchant_entry entry[ENCHANT_MAX_ON_ITEM];
    int           count;
} enchant_set;

// static definition of an enchant kind. lives in the registry, never mutated
// after load.
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
