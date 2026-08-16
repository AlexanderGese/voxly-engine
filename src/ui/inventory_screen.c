#include "inventory_screen.h"
#include "../world/block.h"

#include <stdio.h>

void inv_screen_init(inventory_screen *is) {
    is->visible = 0;
    is->cursor = 0;
}

void inv_screen_toggle(inventory_screen *is) { is->visible = !is->visible; }

void inv_screen_up(inventory_screen *is) {
    if (!is->visible) return;
    is->cursor = (is->cursor + HOTBAR_SLOTS - 1) % HOTBAR_SLOTS;
}

void inv_screen_down(inventory_screen *is) {
    if (!is->visible) return;
    is->cursor = (is->cursor + 1) % HOTBAR_SLOTS;
}

void inv_screen_draw(const inventory_screen *is, const inventory *inv,
                     text_renderer *t, int sw, int sh) {
    if (!is->visible) return;
    int bx = sw / 2 - 60;
    int by = sh / 2 - 80;
    text_draw(t, "== inventory ==", bx, by, 1, 1, 1, sw, sh);

    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        char buf[64];
        block_id id = inv->slot[i];
        const char *name = block_get(id)->name;
        snprintf(buf, sizeof buf, "%s %d. %s",
                 i == is->cursor ? ">" : " ",
                 i + 1, name);
        text_draw(t, buf, bx, by + 16 + i * 10, 1, 1, 1, sw, sh);
    }
}
