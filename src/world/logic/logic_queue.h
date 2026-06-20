#ifndef WORLD_LOGIC_QUEUE_H
#define WORLD_LOGIC_QUEUE_H

#include "logic_types.h"

// scheduled logic updates. when a cell's input changes it doesn't recompute
// immediately - it schedules itself (and possibly a delay for repeaters) onto
// this queue, and the net drains the queue once per logic tick. this is what
// makes repeater chains tick in order and keeps a clock from recursing
// infinitely inside one frame.
//
// it's a binary min-heap keyed by the absolute tick the entry is due. a flat
// array, classic sift up/down. dedup is best-effort and handled by the net via
// the cell's STAGED flag, not here.

#define LOGIC_QUEUE_CAP 16384

typedef struct {
    uint64_t key;       // logic_key of the target cell
    uint32_t due_tick;  // absolute tick this fires on
    uint32_t seq;       // insertion order, breaks ties so it stays FIFO-ish
} logic_event;

typedef struct {
    logic_event heap[LOGIC_QUEUE_CAP];
    int         len;
    uint32_t    seq;    // monotonically increasing stamp
} logic_queue;

void logic_queue_init(logic_queue *q);

// schedule a cell update to fire at (now + delay). delay 0 means next drain.
// returns 0 if the queue is full (the update is silently dropped - better a
// missed tick than a smashed stack).
int  logic_queue_push(logic_queue *q, uint64_t key, uint32_t now, uint32_t delay);

// peek the tick of the earliest event, or UINT32_MAX if empty.
uint32_t logic_queue_peek_tick(const logic_queue *q);

// pop the earliest event into *out. returns 0 if empty.
int  logic_queue_pop(logic_queue *q, logic_event *out);

int  logic_queue_empty(const logic_queue *q);
int  logic_queue_len(const logic_queue *q);
void logic_queue_clear(logic_queue *q);

#endif
