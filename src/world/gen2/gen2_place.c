#include "gen2_place.h"

void gen2_place_init(gen2_place_buf *b, gen2_placement *storage, int cap) {
    if (!b) return;
    b->items = storage;
    b->cap   = (storage && cap > 0) ? cap : 0;
    b->count = 0;
}

int gen2_place_add(gen2_place_buf *b, int x, int y, int z, block_id id) {
    if (!b || !b->items) return 0;
    if (b->count >= b->cap) return 0;   // full, drop it
    gen2_placement *p = &b->items[b->count++];
    p->x = x;
    p->y = y;
    p->z = z;
    p->id = id;
    return 1;
}

void gen2_place_reset(gen2_place_buf *b) {
    if (b) b->count = 0;
}
