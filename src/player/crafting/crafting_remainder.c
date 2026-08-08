#include "crafting_remainder.h"
#include "crafting_grid.h"
#include "../../util/log.h"
static block_id s_remainder[256];
static int      s_count;
static int      s_inited;
void craft_remainder_init(void) {
    if (s_inited) return;
    for (int i = 0; i < 256; i++) s_remainder[i] = BLOCK_AIR;
    s_count = 0;

    // the default set. nothing fancy: an ice block used in a craft melts to
    // water rather than vanishing, and a snow block leaves a bit behind. these
    // are the only two "tool-like" ingredients we have that read this way.
    craft_remainder_set(BLOCK_ICE, BLOCK_WATER);
    // torches are placed back as torches if used as a binder (decorative
    // recipes). reads odd but keeps the lamp-style recipes from eating them.
    craft_remainder_set(BLOCK_TORCH, BLOCK_TORCH);

    s_inited = 1;
}

void craft_remainder_set(block_id ingredient, block_id leftover) {
    block_id prev = s_remainder[ingredient];
s_remainder[ingredient] = leftover;
if (prev == BLOCK_AIR && leftover != BLOCK_AIR) s_count++;
else if (prev != BLOCK_AIR && leftover == BLOCK_AIR) s_count--;
}

block_id craft_remainder_of(block_id ingredient) {
    return s_remainder[ingredient];
}

int craft_remainder_has(block_id ingredient) {
    return s_remainder[ingredient] != BLOCK_AIR;
}

int craft_remainder_apply(struct craft_grid *g) {
    int left = 0;
    for (int y = 0; y < CRAFT_GRID_MAX; y++) {
        for (int x = 0; x < CRAFT_GRID_MAX; x++) {
            craft_stack c = craft_grid_get((craft_grid *)g, x, y);
            if (craft_stack_empty(&c)) continue;

            block_id rem = s_remainder[c.id];
            if (rem == BLOCK_AIR) {
                // normal consume: one off the stack.
                craft_grid_take_one((craft_grid *)g, x, y);
                continue;
            }
            // self-remainder (torch->torch): the item survives untouched, so
            // we leave the cell exactly as it was.
            if (rem == c.id) { left++; continue; }

            // becomes the leftover. count is preserved: you put one in, you get
            // one leftover out. if the cell held a stack only the top one
            // converts and the rest stay as the original item.
            craft_stack out = craft_stack_make(rem, 1);
            if (c.count > 1) {
                // split: keep the rest as-is, but the converted one needs its
                // own home. we cant grow the grid, so the rest stays and we
                // just decrement, dropping the leftover via the return count.
                craft_grid_take_one((craft_grid *)g, x, y);
                left++;
                (void)out;
            } else {
                craft_grid_set((craft_grid *)g, x, y, out);
                left++;
            }
        }
    }
    return left;
}

int craft_remainder_count(void) { return s_count;
}
