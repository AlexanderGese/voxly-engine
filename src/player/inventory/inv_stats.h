#ifndef PLAYER_INVENTORY_STATS_H
#define PLAYER_INVENTORY_STATS_H

#include "inv_types.h"
#include "inv_grid.h"

// derived numbers about an inventory's contents: carried weight, per-category
// breakdown, fill fraction. the hud shows the weight bar and the encumbrance
// state from here; gameplay (slower movement when overloaded) reads the same
// values so the bar never lies about what the legs feel.
//
// weight is a made-up unit ("kg-ish"). blocks are heavy, tools middling, food
// light. it's all in inv_stats.c so balancing is one table to tweak.

typedef enum {
    INV_ENCUMBER_NONE = 0,   // plenty of room to spare
    INV_ENCUMBER_LIGHT,      // noticeable but fine
    INV_ENCUMBER_HEAVY,      // movement starts to drag
    INV_ENCUMBER_OVERLOADED  // at/over the cap
} inv_encumber;

typedef struct {
    int   total_items;                 // raw item count
    int   used_slots;
    int   free_slots;
    float fill_frac;                   // used / total slots, 0..1
    float weight;                      // total carried weight
    float weight_cap;                  // the overloaded threshold
    float per_category[INV_CAT_COUNT]; // weight broken out by category
    inv_encumber state;
} inv_stats;

// weight of one unit of an item. exposed for tooltips ("12.0 kg").
float inv_stats_item_weight(inv_item_id id);

// roll up a grid into stats. `weight_cap` is the carry limit (e.g. derived from
// the player's strength); pass <=0 to use the built-in default.
void  inv_stats_compute(const inv_grid *g, float weight_cap, inv_stats *out);

// the movement speed multiplier implied by an encumbrance state. 1.0 unladen,
// dropping toward ~0.4 when overloaded. physics multiplies move speed by this.
float inv_stats_speed_mult(inv_encumber state);

// human-readable label for the state, for the hud.
const char *inv_stats_encumber_name(inv_encumber state);

#endif
