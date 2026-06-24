#include "rivers_heap.h"

void rivers_heap_init(rivers_heap *h) {
    h->count = 0;
    h->seq   = 0;
}

int rivers_heap_empty(const rivers_heap *h) {
    return h->count == 0;
}

// node a sorts before node b if it's lower, or equal-height but older.
static int node_less(const rivers_heap_node *a, const rivers_heap_node *b) {
    if (a->key != b->key) return a->key < b->key;
    return a->seq < b->seq;
}

static void swap_nodes(rivers_heap_node *a, rivers_heap_node *b) {
    rivers_heap_node t = *a; *a = *b; *b = t;
}

void rivers_heap_push(rivers_heap *h, int cell, int key) {
    if (h->count >= RIVERS_CELLS) return;   // cant happen, but dont smash memory
    int i = h->count++;
    h->nodes[i].cell = cell;
    h->nodes[i].key  = key;
    h->nodes[i].seq  = h->seq++;

    // sift up
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (node_less(&h->nodes[i], &h->nodes[parent])) {
            swap_nodes(&h->nodes[i], &h->nodes[parent]);
            i = parent;
        } else {
            break;
        }
    }
}

int rivers_heap_pop(rivers_heap *h, int *out_key) {
    rivers_heap_node top = h->nodes[0];
    h->count--;
    if (h->count > 0) {
        h->nodes[0] = h->nodes[h->count];
        // sift down
        int i = 0;
        for (;;) {
            int l = 2 * i + 1, r = 2 * i + 2, best = i;
            if (l < h->count && node_less(&h->nodes[l], &h->nodes[best])) best = l;
            if (r < h->count && node_less(&h->nodes[r], &h->nodes[best])) best = r;
            if (best == i) break;
            swap_nodes(&h->nodes[i], &h->nodes[best]);
            i = best;
        }
    }
    if (out_key) *out_key = top.key;
    return top.cell;
}
