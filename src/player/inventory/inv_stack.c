#include "inv_stack.h"
#include "inv_registry.h"

// keep a stack honest: if it ever hits zero, it becomes a real empty (id
// cleared too) so downstream is_empty checks stay cheap.
static void normalize(inv_stack *s) {
    if (s->count == 0) s->id = INV_ITEM_NONE;
}

int inv_stack_is_empty(const inv_stack *s) {
    return s->id == INV_ITEM_NONE || s->count == 0;
}

int inv_stack_is_full(const inv_stack *s) {
    if (inv_stack_is_empty(s)) return 0;
    return s->count >= inv_item_max_stack(s->id);
}

uint16_t inv_stack_space_for(const inv_stack *s, inv_item_id id) {
    uint16_t max = inv_item_max_stack(id);
    if (inv_stack_is_empty(s)) return max;
    if (s->id != id) return 0;
    return (s->count >= max) ? 0 : (uint16_t)(max - s->count);
}

uint16_t inv_stack_space(const inv_stack *s) {
    if (inv_stack_is_empty(s)) return 0;   // empty has no item to size against
    return inv_stack_space_for(s, s->id);
}

int inv_stack_can_merge(const inv_stack *a, const inv_stack *b) {
    if (inv_stack_is_empty(b)) return 0;
    if (inv_stack_is_empty(a)) return 1;            // empty soaks anything
    return a->id == b->id && !inv_stack_is_full(a);
}

uint16_t inv_stack_merge(inv_stack *dst, inv_stack *src) {
    if (inv_stack_is_empty(src)) return 0;
    if (!inv_stack_is_empty(dst) && dst->id != src->id) return 0;

    if (inv_stack_is_empty(dst)) dst->id = src->id;

    uint16_t room = inv_stack_space_for(dst, src->id);
    uint16_t move = src->count < room ? src->count : room;

    dst->count = (uint16_t)(dst->count + move);
    src->count = (uint16_t)(src->count - move);
    normalize(src);
    normalize(dst);
    return move;
}

uint16_t inv_stack_transfer(inv_stack *dst, inv_stack *src, uint16_t n) {
    if (inv_stack_is_empty(src) || n == 0) return 0;
    if (!inv_stack_is_empty(dst) && dst->id != src->id) return 0;

    if (inv_stack_is_empty(dst)) dst->id = src->id;

    uint16_t room = inv_stack_space_for(dst, src->id);
    uint16_t avail = src->count;
    uint16_t move = n;
    if (move > room)  move = room;
    if (move > avail) move = avail;

    dst->count = (uint16_t)(dst->count + move);
    src->count = (uint16_t)(src->count - move);
    normalize(src);
    normalize(dst);
    return move;
}

uint16_t inv_stack_split_half(inv_stack *src, inv_stack *out) {
    if (inv_stack_is_empty(src) || !inv_stack_is_empty(out)) return 0;
    uint16_t half = (uint16_t)((src->count + 1) / 2);   // round up to the held hand
    out->id    = src->id;
    out->count = half;
    src->count = (uint16_t)(src->count - half);
    normalize(src);
    return half;
}

int inv_stack_take_one(inv_stack *src, inv_stack *out) {
    if (inv_stack_is_empty(src)) return 0;
    if (!inv_stack_is_empty(out) && out->id != src->id) return 0;
    if (!inv_stack_is_empty(out) && inv_stack_is_full(out)) return 0;

    if (inv_stack_is_empty(out)) out->id = src->id;
    out->count++;
    src->count--;
    normalize(src);
    return 1;
}

void inv_stack_swap(inv_stack *a, inv_stack *b) {
    inv_stack t = *a;
    *a = *b;
    *b = t;
}
