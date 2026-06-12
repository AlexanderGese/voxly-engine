#ifndef WORLD_BLOCK_UPDATE_H
#define WORLD_BLOCK_UPDATE_H

#include "world.h"

// scheduled block updates. when a block changes (broken, placed, liquid flow),
// neighboring blocks may need to react on the next tick. this queue stores
// pending updates and processes them in FIFO order.

#define BLOCK_UPDATE_MAX 4096

typedef struct {
    int wx, wy, wz;
    int tick_delay;  // how many ticks to wait before processing
} block_update;

typedef struct {
    block_update queue[BLOCK_UPDATE_MAX];
    int head, tail;
} block_update_queue;

void buq_init(block_update_queue *q);
void buq_schedule(block_update_queue *q, int wx, int wy, int wz, int delay);
void buq_tick(block_update_queue *q, world *w);
int  buq_pending(const block_update_queue *q);

#endif
