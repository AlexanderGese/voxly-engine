#ifndef UI_INVSCREEN_SLOT_H
#define UI_INVSCREEN_SLOT_H

// the slot model. a slot is just a (block_id, count) pair with the convention
// that count==0 means empty regardless of what id leftover sits in `block`.
// keeping the leftover id around is handy when the count momentarily hits zero
// mid-drag and we want to remember what was there. nothing reads it once empty.

#include "../../world/block.h"
#include "invscreen_config.h"

typedef struct {
    block_id block;
    int      count;
} invscreen_slot;

static inline invscreen_slot invscreen_slot_empty(void) {
    invscreen_slot s = { BLOCK_AIR, 0 };
    return s;
}

static inline int invscreen_slot_is_empty(const invscreen_slot *s) {
    return s->count <= 0 || s->block == BLOCK_AIR;
}

static inline invscreen_slot invscreen_slot_make(block_id id, int n) {
    invscreen_slot s;
    if (id == BLOCK_AIR || n <= 0) return invscreen_slot_empty();
    s.block = id;
    s.count = n > INVSCR_STACK_MAX ? INVSCR_STACK_MAX : n;
    return s;
}

// room left before the stack caps out. empty slots return the full max.
static inline int invscreen_slot_room(const invscreen_slot *s) {
    if (invscreen_slot_is_empty(s)) return INVSCR_STACK_MAX;
    return INVSCR_STACK_MAX - s->count;
}

// can `src` stack onto `dst`? same id and dst not full. empty dst always yes.
static inline int invscreen_slot_stackable(const invscreen_slot *dst,
                                           const invscreen_slot *src) {
    if (invscreen_slot_is_empty(src)) return 0;
    if (invscreen_slot_is_empty(dst)) return 1;
    return dst->block == src->block && dst->count < INVSCR_STACK_MAX;
}

// move up to `n` items from src into dst, respecting the stack cap and id match.
// mutates both. returns how many actually moved. this is the one primitive the
// drag/split logic is built on, so it tries hard to never lose an item.
static inline int invscreen_slot_transfer(invscreen_slot *dst,
                                          invscreen_slot *src, int n) {
    if (n <= 0) return 0;
    if (invscreen_slot_is_empty(src)) return 0;
    if (n > src->count) n = src->count;

    if (invscreen_slot_is_empty(dst)) {
        int moved = n > INVSCR_STACK_MAX ? INVSCR_STACK_MAX : n;
        dst->block = src->block;
        dst->count = moved;
        src->count -= moved;
        if (src->count <= 0) *src = invscreen_slot_empty();
        return moved;
    }

    if (dst->block != src->block) return 0;
    int room = INVSCR_STACK_MAX - dst->count;
    int moved = n < room ? n : room;
    dst->count += moved;
    src->count -= moved;
    if (src->count <= 0) *src = invscreen_slot_empty();
    return moved;
}

// swap two slots wholesale. used when ids differ and you cant stack.
static inline void invscreen_slot_swap(invscreen_slot *a, invscreen_slot *b) {
    invscreen_slot t = *a;
    *a = *b;
    *b = t;
}

#endif
