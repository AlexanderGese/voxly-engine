#ifndef WORLD_COLORLIGHT_QUEUE_H
#define WORLD_COLORLIGHT_QUEUE_H

#include <stdint.h>

// fixed ring buffer of flood nodes, same shape as lighting.c's queue but each
// node also carries which channel it's for and a "was" level used by the
// removal pass. one queue type serves both add and remove floods.
//
// shared static storage, NOT reentrant. the flood is single threaded and runs
// to completion before anything else touches it, so a couple of module-static
// rings is plenty and saves us mallocing per relight.

#define COLORLIGHT_QMAX 65536

typedef struct {
    int     x, y, z;   // world coords
    uint8_t chan;      // 0=r 1=g 2=b
    uint8_t level;     // current level for add; "level it had" for remove
} colorlight_qnode;

typedef struct {
    colorlight_qnode *buf;   // points at one of the static rings
    int head, tail;
    int cap;
} colorlight_queue;

// grab the two shared rings (add ring, remove ring). they're distinct so a
// removal can re-light from survivors without trampling its own remove queue.
colorlight_queue *colorlight_queue_add(void);
colorlight_queue *colorlight_queue_remove(void);

void colorlight_queue_reset(colorlight_queue *q);
int  colorlight_queue_empty(const colorlight_queue *q);
int  colorlight_queue_count(const colorlight_queue *q);

// push/pop. push drops silently when full (matches lighting.c "cheap n dirty").
void colorlight_queue_push(colorlight_queue *q, int x, int y, int z, uint8_t chan, uint8_t level);
int  colorlight_queue_pop(colorlight_queue *q, colorlight_qnode *out);

#endif
