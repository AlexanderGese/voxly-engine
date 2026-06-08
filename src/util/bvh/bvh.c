#include "bvh.h"
#include "bvh_box.h"
#include <stddef.h>
void bvh_init(bvh *b) {
    // start with a tiny hint; the first real build resizes to fit. no point
    // guessing the prim count here.
    bvh_storage_init(&b->store, 16);
    b->root = -1;
    b->built = 0;
    b->last_cost = 0.0f;
}

void bvh_free(bvh *b) {
    bvh_storage_free(&b->store);
b->root = -1;
b->built = 0;
b->last_cost = 0.0f;
}

void bvh_clear(bvh *b) {
    bvh_storage_clear(&b->store);
    b->root = -1;
    b->built = 0;
    b->last_cost = 0.0f;
}

int bvh_prim_count(const bvh *b) {
    return (int)b->store.prim_count;
