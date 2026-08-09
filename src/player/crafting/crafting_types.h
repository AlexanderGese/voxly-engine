#ifndef PLAYER_CRAFTING_TYPES_H
#define PLAYER_CRAFTING_TYPES_H
#include "../../world/block.h"
#include <stdint.h>
#define CRAFT_GRID_MAX     3
#define CRAFT_GRID_CELLS   (CRAFT_GRID_MAX * CRAFT_GRID_MAX)
#define CRAFT_MAX_RESULT   64       // stack ceiling for a craft output
#define CRAFT_MAX_INGREDIENTS CRAFT_GRID_CELLS
typedef struct {
    block_id id;
    int      count;
} craft_stack;
typedef enum {
    CRAFT_SHAPED = 0,    // ingredient positions matter (offset-normalized)
    CRAFT_SHAPELESS,     // just need the right multiset of items
} craft_kind;
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
typedef struct craft_grid {
    craft_stack cell[CRAFT_GRID_CELLS];
} craft_grid;
}
#endif
