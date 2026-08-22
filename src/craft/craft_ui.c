#include "craft_ui.h"
#include "../world/block.h"

#include <stdio.h>

void craft_ui_draw(const crafting_grid *g, text_renderer *t, int sw, int sh) {
    if (!g->open) return;

    int bx = sw / 2 - 60;
    int by = sh / 2 - 60;
    text_draw(t, "crafting", bx, by - 14, 1, 1, 1, sw, sh);

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            char buf[8];
            block_id id = g->grid[y][x];
            snprintf(buf, sizeof buf, "%s%02d",
                     (x == g->cursor_x && y == g->cursor_y) ? ">" : " ",
                     id);
            text_draw(t, buf, bx + x * 24, by + y * 14,
                      1, 1, 1, sw, sh);
        }
    }

    char out[32];
    if (g->output != BLOCK_AIR) {
        snprintf(out, sizeof out, "= %s x%d", block_get(g->output)->name, g->output_count);
    } else {
        snprintf(out, sizeof out, "= (none)");
    }
    text_draw(t, out, bx, by + 14 * 3 + 8, 1, 1, 0.5f, sw, sh);
}
