#ifndef WORLD_LOGIC_PROPAGATE_H
#define WORLD_LOGIC_PROPAGATE_H

#include "logic_types.h"
#include "logic_grid.h"

// wire propagation. given the current output of every emitter (sources, lit
// torches, gates, committed repeaters), recompute the power level carried by
// every wire cell via a saturating multi-source breadth-first flood.
//
// this is the per-tick "settle the dust" pass. emitters are fixed points; only
// wire cells get rewritten. sinks (lamps) read their neighbours afterwards.
//
// the flood seeds a queue with every wire neighbour of an emitter at
// (emit - 1), then relaxes outward taking the max at each cell, exactly like a
// shortest-path / light flood. converges because power strictly decreases.

// recompute every wire cell's power in the grid. writes results straight into
// each wire cell's ->power (this is the committed value, not staged - wires
// have no delay). returns the number of wire cells that changed level, so the
// caller can decide whether downstream gates/lamps need re-evaluation.
int logic_propagate_wires(logic_grid *g);

// the level a single wire cell should hold, computed locally from its already
// settled neighbours. handy for incremental pokes and for unit tests; the full
// flood is the authoritative version.
uint8_t logic_propagate_sample(logic_grid *g, const logic_cell *wire);

#endif
