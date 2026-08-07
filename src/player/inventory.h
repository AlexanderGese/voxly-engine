#ifndef PLAYER_INVENTORY_H
#define PLAYER_INVENTORY_H

#include "../world/block.h"
#include "../config.h"

typedef struct {
    block_id slot[HOTBAR_SLOTS];
    int      selected;
} inventory;

void inventory_init(inventory *inv);
void inventory_select(inventory *inv, int idx);
void inventory_scroll(inventory *inv, int delta);
block_id inventory_current(const inventory *inv);
int  inventory_add(inventory *inv, block_id id);  // returns 1 if added

#endif
