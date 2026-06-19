#ifndef WORLD_LIGHTPROP_QUEUE_H
#define WORLD_LIGHTPROP_QUEUE_H

#include "lightprop_types.h"

// fixed-capacity ring buffer of lp_nodes. lighting.c had one of these stuffed
// into a file-static; we want two live at once (add queue + removal queue) plus
// reentrancy, so it's a real struct now.
//
// overflow policy: drop. a dropped node just means that corner of the world
// stays a little too dark/bright until the next full edit nudges it. acceptable
// for a game, would be a sin in a renderer.

#define LP_QCAP 65536

typedef struct {
    lp_node buf[LP_QCAP];
    int head;
    int tail;
    int dropped;   // diagnostics. if this climbs, bump LP_QCAP.
} lp_queue;

void lp_queue_init(lp_queue *q);
void lp_queue_reset(lp_queue *q);

int  lp_queue_empty(const lp_queue *q);
int  lp_queue_count(const lp_queue *q);

// push a node. returns 1 if stored, 0 if the ring was full (bumps dropped).
int  lp_queue_push(lp_queue *q, int x, int y, int z, uint8_t level);

// pop into *out. returns 0 if empty.
int  lp_queue_pop(lp_queue *q, lp_node *out);

#endif
