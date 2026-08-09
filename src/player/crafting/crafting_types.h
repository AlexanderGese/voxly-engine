#ifndef PLAYER_CRAFTING_TYPES_H
#define PLAYER_CRAFTING_TYPES_H

#include "../../world/block.h"
#include <stdint.h>

// shared vocabulary for the crafting module. everything else includes this.
// a "stack" is just (block_id, count). empty == BLOCK_AIR / count 0.
// the grid is up to 3x3, but small recipes can match anywhere inside it.

#define CRAFT_GRID_MAX     3
#define CRAFT_GRID_CELLS   (CRAFT_GRID_MAX * CRAFT_GRID_MAX)
#define CRAFT_MAX_RESULT   64       // stack ceiling for a craft output

// recipe ingredient list never gets bigger than the grid itself.
#define CRAFT_MAX_INGREDIENTS CRAFT_GRID_CELLS

typedef struct {
    block_id id;
    int      count;
} craft_stack;

typedef enum {
    CRAFT_SHAPED = 0,    // ingredient positions matter (offset-normalized)
    CRAFT_SHAPELESS,     // just need the right multiset of items
} craft_kind;

// a single recipe. for shaped recipes `pattern` holds the WxH grid of ids,
// row-major, with BLOCK_AIR meaning "this cell must be empty". for shapeless
// recipes we only look at `ing`/`ing_count`.
typedef struct {
    craft_kind  kind;
    int         id;          // stable recipe id, index into the book

    // shaped layout
    int         w, h;        // bounding box of the pattern, 1..3
    block_id    pattern[CRAFT_GRID_CELLS];

    // shapeless ingredients (also filled for shaped, for the book listing)
    block_id    ing[CRAFT_MAX_INGREDIENTS];
    int         ing_n;

    craft_stack result;
    const char *name;        // for the recipe book ui
} craft_recipe;

// a 3x3 working grid. row-major, [y*3 + x].
typedef struct craft_grid {
    craft_stack cell[CRAFT_GRID_CELLS];
} craft_grid;

static inline int craft_stack_empty(const craft_stack *s) {
    return s->id == BLOCK_AIR || s->count <= 0;
}

static inline craft_stack craft_stack_make(block_id id, int n) {
    craft_stack s = { id, n };
    if (n <= 0) { s.id = BLOCK_AIR; s.count = 0; }
    return s;
}

#endif
