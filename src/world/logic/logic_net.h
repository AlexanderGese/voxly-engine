#ifndef WORLD_LOGIC_NET_H
#define WORLD_LOGIC_NET_H

#include "logic_types.h"
#include "logic_grid.h"
#include "logic_queue.h"
#include "logic_clock.h"

// the whole wire-logic network for a world. owns the sparse cell grid and the
// scheduled-update queue, and drives one logic tick at a time. the game loop
// calls logic_net_tick at a fixed logic rate (a few hz, not per-frame).
//
// a tick does, in order:
// 1. drain every event due this tick, re-evaluating those cells
// 2. settle the wire flood so dust carries the new levels
// 3. re-check gates/repeaters/lamps that border changed wires
// 4. commit staged outputs and wake downstream cells for the next tick
//
// the queue keeps it from recomputing the whole grid every tick: only the
// cells that were poked do work.

typedef struct {
    logic_grid  grid;
    logic_queue queue;
    logic_clock clock;       // runaway-clock detector / throttle
    uint32_t    tick;        // monotonically increasing logic tick
    int         dirty_all;   // force a full settle next tick (after bulk edits)
    int         last_active; // cells touched on the last tick, for debug
} logic_net;

void logic_net_init(logic_net *net);
void logic_net_free(logic_net *net);

// add / update / remove a logic block at a world coord. these schedule the
// affected neighbourhood so the change ripples out on the next tick.
logic_cell *logic_net_place(logic_net *net, int x, int y, int z,
                            block_id kind, logic_dir facing);
void        logic_net_remove(logic_net *net, int x, int y, int z);

// toggle a lever / press a button / set a source on or off at a coord. no-op if
// the cell there isn't a controllable source. button auto-resets after a hold.
void logic_net_set_source(logic_net *net, int x, int y, int z, int on);
void logic_net_press(logic_net *net, int x, int y, int z);

// wake a coord: schedule it (and its neighbours) for re-evaluation. used by the
// world glue when an adjacent non-logic block changes.
void logic_net_wake(logic_net *net, int x, int y, int z, uint32_t delay);

// advance the network by exactly one logic tick. returns the number of cells
// whose output changed (0 means the network is at rest).
int  logic_net_tick(logic_net *net);

// is the network quiescent (nothing scheduled)? handy to skip ticks entirely.
int  logic_net_at_rest(const logic_net *net);

#endif
