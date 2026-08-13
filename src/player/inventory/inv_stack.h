#ifndef PLAYER_INVENTORY_STACK_H
#define PLAYER_INVENTORY_STACK_H

#include "inv_types.h"

// stack-level arithmetic. no grids here, just two inv_stack* and the rules
// that bind them. everything is in-place; callers pass pointers into a grid.
// these never look at the registry directly except through max_stack so the
// rules stay in one place.

// the empty/none stack value. handy for clearing.
static inline inv_stack inv_stack_empty(void) {
    inv_stack s = { INV_ITEM_NONE, 0 };
    return s;
}

int  inv_stack_is_empty(const inv_stack *s);
int  inv_stack_is_full(const inv_stack *s);      // count >= item max_stack

// remaining headroom on a stack before it hits its item's max_stack. an empty
// stack reports the full max for the queried item id.
uint16_t inv_stack_space(const inv_stack *s);
uint16_t inv_stack_space_for(const inv_stack *s, inv_item_id id);

// can b be poured into a (same item, a not full)? empty a accepts anything.
int  inv_stack_can_merge(const inv_stack *a, const inv_stack *b);

// pour from src into dst up to dst's cap. returns how many actually moved.
// updates both. if dst is empty it adopts src's id first.
uint16_t inv_stack_merge(inv_stack *dst, inv_stack *src);

// move exactly `n` (clamped to what's available and what fits) src->dst.
// returns the count moved. used by shift-click / drag-one paths.
uint16_t inv_stack_transfer(inv_stack *dst, inv_stack *src, uint16_t n);

// split half (rounded up) of src into out. out must be empty. returns moved.
uint16_t inv_stack_split_half(inv_stack *src, inv_stack *out);

// pull exactly one off src into out (out empty or same item). returns 1/0.
int  inv_stack_take_one(inv_stack *src, inv_stack *out);

// swap two stacks wholesale.
void inv_stack_swap(inv_stack *a, inv_stack *b);

#endif
