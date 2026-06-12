#ifndef WORLD_CONTAINER_H
#define WORLD_CONTAINER_H

#include "block.h"
#include <stdint.h>

// chest-like containers. stored per-block in a side hashmap keyed by
// packed (chunk, lx, ly, lz).

#define CONTAINER_SLOTS 27

typedef struct {
    block_id slot[CONTAINER_SLOTS];
    int      count[CONTAINER_SLOTS];
} container;

void       containers_init(void);
container *container_get_or_create(int wx, int wy, int wz);
container *container_peek(int wx, int wy, int wz);
void       container_delete(int wx, int wy, int wz);
int        container_add(container *c, block_id id, int amount);
int        container_remove_one(container *c, int slot_idx, block_id *out);

#endif
