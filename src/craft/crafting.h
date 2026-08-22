#ifndef CRAFT_CRAFTING_H
#define CRAFT_CRAFTING_H
#include "recipe.h"
typedef struct {
    int      open;
    int      cursor_x, cursor_y;   // which grid cell is selected
    block_id grid[GRID_SIZE][GRID_SIZE];
    block_id output;
    int      output_count;
} crafting_grid;
void crafting_init(crafting_grid *c);
void crafting_toggle(crafting_grid *c);
void crafting_place(crafting_grid *c, block_id id);
#endif
