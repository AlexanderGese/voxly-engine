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
int logic_propagate_wires(logic_grid *g);
uint8_t logic_propagate_sample(logic_grid *g, const logic_cell *wire);
#endif
