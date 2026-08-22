#ifndef CRAFT_CRAFTING_H
#define CRAFT_CRAFTING_H

#include "recipe.h"

// a 3x3 crafting grid + output slot. open with 'e'.

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
void crafting_clear(crafting_grid *c);
void crafting_update(crafting_grid *c);
int  crafting_take_output(crafting_grid *c, block_id *out, int *count);
void crafting_move_cursor(crafting_grid *c, int dx, int dy);

#endif
