#ifndef PLAYER_INVENTORY_TYPES_H
#define PLAYER_INVENTORY_TYPES_H
#include <stdint.h>
#include "../../world/block.h"
// plain-data types for the full player inventory. the old inventory.h in the
// parent dir is just a 9-slot hotbar of block ids; this one is the real thing
// with stacks, item ids decoupled from block ids, and a slot grid you can
// merge/split/move around. nothing in here allocates or touches render, so the
// ui layer and the save layer can both lean on it.
//
// everything here is prefixed inv_ so it never argues with the legacy hotbar.
typedef uint16_t inv_item_id;
// item 0 is always "nothing". keeps empty-slot checks to a single compare.
#define INV_ITEM_NONE   ((inv_item_id)0)
// hard ceiling on a single stack regardless of the item's own max. a slot
// count is a uint16 so we never overflow, but realistically items cap way
// lower (see the registry).
#define INV_STACK_HARD_CAP   999
// how the main grids are shaped. the player window is 3 rows of 9 over a
// hotbar row of 9, classic. these are independent of the legacy HOTBAR_SLOTS
// in config.h on purpose; that constant is for the old struct.
#define INV_COLS             9
#define INV_MAIN_ROWS        3
#define INV_HOTBAR_SLOTS     INV_COLS
#define INV_MAIN_SLOTS       (INV_COLS * INV_MAIN_ROWS)
#define INV_TOTAL_SLOTS      (INV_MAIN_SLOTS + INV_HOTBAR_SLOTS)
// one occupied or empty slot. an empty slot is id==NONE *and* count==0; we
// keep both pinned in lockstep so a stray count on an empty slot can't leak.
typedef struct {
    inv_item_id id;
    uint16_t    count;
} inv_stack;
// a rectangular bag of slots. rows*cols == used slots, but we keep them flat
// and let callers do the index math via inv_grid_at().
typedef struct {
    inv_stack *slots;     // owned, rows*cols of them
    int        rows;
    int        cols;
    int        count;     // rows*cols, cached so we stop recomputing it
} inv_grid;
// item category. drives stacking rules and (later) tooltips. tools and armor
// don't stack; blocks and materials do.
typedef enum {
    INV_CAT_BLOCK = 0,
    INV_CAT_MATERIAL,
    INV_CAT_TOOL,
    INV_CAT_FOOD,
    INV_CAT_ARMOR,
    INV_CAT_MISC,
    INV_CAT_COUNT
} inv_category;
// static description of an item kind. the registry owns the table; callers
// only ever borrow const pointers into it.
typedef struct {
    const char  *name;
    inv_category category;
    uint16_t     max_stack;   // clamped against INV_STACK_HARD_CAP at register
    block_id     place_block; // BLOCK_AIR if not placeable
    int          atlas_tile;  // icon tile in the item atlas, -1 = derive from block
    uint16_t     durability;  // 0 for stackables, >0 for tools/armor
} inv_item_def;
// result codes shared by the move/merge ops so the ui can react (play a sound,
typedef enum {
    INV_OK = 0,
    INV_RESULT_NOOP,       // nothing changed
    INV_RESULT_MERGED,     // src folded into dst
    INV_RESULT_PARTIAL,    // some moved, some left behind (dst capped out)
    INV_RESULT_SWAPPED,    // two different stacks traded places
    INV_RESULT_FULL,       // no room at all
    INV_RESULT_INVALID     // bad slot index / mismatched item
} inv_result;
#endif
