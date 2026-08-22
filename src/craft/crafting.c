#include "crafting.h"
#include "../world/block.h"

#include <string.h>

void crafting_init(crafting_grid *c) {
    memset(c, 0, sizeof *c);
}

void crafting_toggle(crafting_grid *c) { c->open = !c->open; }

void crafting_place(crafting_grid *c, block_id id) {
    if (!c->open) return;
    c->grid[c->cursor_y][c->cursor_x] = id;
    crafting_update(c);
}

void crafting_clear(crafting_grid *c) {
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            c->grid[y][x] = BLOCK_AIR;
    c->output = BLOCK_AIR;
    c->output_count = 0;
}

void crafting_update(crafting_grid *c) {
    const recipe *r = recipes_match((const block_id (*)[3])c->grid);
    if (r) {
        c->output = r->result;
        c->output_count = r->count;
    } else {
        c->output = BLOCK_AIR;
        c->output_count = 0;
    }
}

int crafting_take_output(crafting_grid *c, block_id *out, int *count) {
    if (c->output == BLOCK_AIR) return 0;
    *out = c->output;
    *count = c->output_count;
    // consume one of each slot
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (c->grid[y][x] != BLOCK_AIR) {
                // we only track ids not counts, so just clear
                c->grid[y][x] = BLOCK_AIR;
            }
        }
    }
    c->output = BLOCK_AIR;
    c->output_count = 0;
    return 1;
}

void crafting_move_cursor(crafting_grid *c, int dx, int dy) {
    c->cursor_x = (c->cursor_x + dx + GRID_SIZE) % GRID_SIZE;
    c->cursor_y = (c->cursor_y + dy + GRID_SIZE) % GRID_SIZE;
}
