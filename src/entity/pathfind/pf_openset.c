#include "pf_openset.h"

void pf_openset_init(pf_openset *h, pf_nodepool *pool) {
    h->len = 0;
    h->pool = pool;
}

int pf_openset_empty(const pf_openset *h) {
    return h->len == 0;
}

// f of the node sitting at heap slot i.
static int slot_f(pf_openset *h, int i) {
    return h->pool->nodes[h->slots[i]].f;
}

// move heap slot i to hold node `idx` and fix the back-pointer.
static void place(pf_openset *h, int i, int idx) {
    h->slots[i] = idx;
    h->pool->nodes[idx].heap_pos = i;
}

static void sift_up(pf_openset *h, int i) {
    int idx = h->slots[i];
    int f   = h->pool->nodes[idx].f;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (slot_f(h, parent) <= f) break;
        place(h, i, h->slots[parent]);
        i = parent;
    }
    place(h, i, idx);
}

static void sift_down(pf_openset *h, int i) {
    int idx = h->slots[i];
    int f   = h->pool->nodes[idx].f;
    for (;;) {
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        int small = i;
        int small_f = f;
        if (l < h->len && slot_f(h, l) < small_f) { small = l; small_f = slot_f(h, l); }
        if (r < h->len && slot_f(h, r) < small_f) { small = r; small_f = slot_f(h, r); }
        if (small == i) break;
        place(h, i, h->slots[small]);
        i = small;
    }
    place(h, i, idx);
}

void pf_openset_push(pf_openset *h, int idx) {
    pf_node *n = &h->pool->nodes[idx];
    if (n->heap_pos >= 0) {
        // already open: f decreased, re-sift from where it sits. since A*
        // only ever lowers f, sifting up is enough.
        sift_up(h, n->heap_pos);
        return;
    }
    if (h->len >= PF_MAX_NODES) return;   // shouldn't happen, pool bounds it
    int i = h->len++;
    place(h, i, idx);
    sift_up(h, i);
}

int pf_openset_pop(pf_openset *h) {
    if (h->len == 0) return -1;
    int top = h->slots[0];
    h->pool->nodes[top].heap_pos = -1;
    h->len--;
    if (h->len > 0) {
        place(h, 0, h->slots[h->len]);
        sift_down(h, 0);
    }
    return top;
}
