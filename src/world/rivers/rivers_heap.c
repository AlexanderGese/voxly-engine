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
    rivers_heap_node t = *a;
*a = *b;
*b = t;
h->count--;
return top.cell;
}
