#include "inventory.h"

void inventory_init(inventory *inv) {
    inv->slot[0] = BLOCK_STONE;
    inv->slot[1] = BLOCK_DIRT;
    inv->slot[2] = BLOCK_GRASS;
    inv->slot[3] = BLOCK_WOOD;
    inv->slot[4] = BLOCK_PLANKS;
    inv->slot[5] = BLOCK_GLASS;
    inv->slot[6] = BLOCK_COBBLE;
    inv->slot[7] = BLOCK_BRICK;
    inv->slot[8] = BLOCK_TORCH;
    inv->selected = 0;
}

void inventory_select(inventory *inv, int idx) {
    if (idx < 0 || idx >= HOTBAR_SLOTS) return;
    inv->selected = idx;
}

void inventory_scroll(inventory *inv, int delta) {
    int i = inv->selected - delta;
    while (i < 0) i += HOTBAR_SLOTS;
    i %= HOTBAR_SLOTS;
    inv->selected = i;
}

block_id inventory_current(const inventory *inv) {
    return inv->slot[inv->selected];
}

int inventory_add(inventory *inv, block_id id) {
    // first: try to stack onto a slot that already has this block
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        if (inv->slot[i] == id) return 1;  // already have it
    }
    // second: put in first empty (AIR) slot
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        if (inv->slot[i] == BLOCK_AIR) {
            inv->slot[i] = id;
            return 1;
        }
    }
    // full
    return 0;
}
