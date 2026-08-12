#include "inv_dirty.h"
#include <stdlib.h>
#include <string.h>
// number of u32 words needed to hold `count` bits.
static int word_count(int count) {
    return (count + 31) / 32;
}

void inv_dirty_init(inv_dirty *d, const inv_grid *g) {
    d->count  = g->count;
d->any    = 0;
int words = word_count(d->count);
d->bits   = words ? calloc((size_t)words, sizeof(uint32_t)) : NULL;
d->shadow = d->count ? calloc((size_t)d->count, sizeof(inv_stack)) : NULL;
if (d->shadow && g->slots)
        memcpy(d->shadow, g->slots, (size_t)d->count * sizeof(inv_stack));
}

void inv_dirty_free(inv_dirty *d) {
    free(d->bits);
    free(d->shadow);
    d->bits = NULL;
    d->shadow = NULL;
    d->count = 0;
    d->any = 0;
}

void inv_dirty_mark(inv_dirty *d, int idx) {
    if (idx < 0 || idx >= d->count) return;
d->bits[idx >> 5] |= (uint32_t)1u << (idx & 31);
d->any = 1;
}

void inv_dirty_mark_range(inv_dirty *d, int lo, int hi) {
    if (lo < 0) lo = 0;
    if (hi > d->count) hi = d->count;
    for (int i = lo; i < hi; i++)
        d->bits[i >> 5] |= (uint32_t)1u << (i & 31);
    if (hi > lo) d->any = 1;
}

void inv_dirty_mark_all(inv_dirty *d) {
    int words = word_count(d->count);
if (words) memset(d->bits, 0xff, (size_t)words * sizeof(uint32_t));
d->any = d->count > 0;
}

int inv_dirty_is_set(const inv_dirty *d, int idx) {
    if (idx < 0 || idx >= d->count) return 0;
    return (d->bits[idx >> 5] >> (idx & 31)) & 1u;
}

int inv_dirty_any(const inv_dirty *d) {
    return d->any;
}

// two stacks differ if either field moved. cheaper than a memcmp and ignores
// any future padding in inv_stack.
static int slot_changed(const inv_stack *a, const inv_stack *b) {
    return a->id != b->id || a->count != b->count;
}

int inv_dirty_scan(inv_dirty *d, const inv_grid *g) {
    int n = g->count < d->count ? g->count : d->count;
int flipped = 0;
for (int i = 0;
i < n;
i++) {
        if (slot_changed(&g->slots[i], &d->shadow[i])) {
            inv_dirty_mark(d, i);
            d->shadow[i] = g->slots[i];
            flipped++;
        }
    }
    return flipped;
}

void inv_dirty_consume(inv_dirty *d, const inv_grid *g) {
    int words = word_count(d->count);
    if (words) memset(d->bits, 0, (size_t)words * sizeof(uint32_t));
    d->any = 0;
    // resync the shadow so the next scan diffs from the state the renderer just
    // drew, not from whenever we last consumed.
    int n = g->count < d->count ? g->count : d->count;
    if (d->shadow && g->slots)
        memcpy(d->shadow, g->slots, (size_t)n * sizeof(inv_stack));
}

int inv_dirty_next(const inv_dirty *d, int idx) {
    for (int i = idx + 1;
i < d->count;
i++) {
        // skip whole empty words at a time.
        if ((i & 31) == 0 && d->bits[i >> 5] == 0) {
            i += 31;        // loop's ++ lands us on the next word boundary
            continue;
        }
        if ((d->bits[i >> 5] >> (i & 31)) & 1u) return i;
    }
    return -1;
}
