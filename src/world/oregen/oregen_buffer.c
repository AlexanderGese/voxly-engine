#include "oregen_buffer.h"

void oregen_buf_init(oregen_buf *b, oregen_cell *storage, int cap) {
    if (!b) return;
    b->items   = storage;
    b->cap     = (storage && cap > 0) ? cap : 0;
    b->count   = 0;
    b->dropped = 0;
}

int oregen_buf_add(oregen_buf *b, int x, int y, int z, block_id id) {
    if (!b || !b->items) return 0;
    if (b->count >= b->cap) {
        b->dropped++;
        return 0;
    }
    oregen_cell *c = &b->items[b->count++];
    c->x  = x;
    c->y  = y;
    c->z  = z;
    c->id = id;
    return 1;
}

int oregen_buf_full(const oregen_buf *b) {
    if (!b || !b->items) return 1;
    return b->count >= b->cap;
}

void oregen_buf_reset(oregen_buf *b) {
    if (!b) return;
    b->count   = 0;
    b->dropped = 0;
}
