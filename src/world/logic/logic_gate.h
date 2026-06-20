#ifndef WORLD_LOGIC_GATE_H
#define WORLD_LOGIC_GATE_H

#include "logic_types.h"
#include "logic_grid.h"

// combinational gates. a gate faces one direction: that face is its single
// output, the two perpendicular faces are its inputs, and the back face is
// ignored. NOT only reads the back face as its input and ignores the sides,
// which is the usual convention for an inverter.
//
// gates restore signal: the output is always LOGIC_MAX or 0, never a partial.

// pure truth table on boolean inputs. a/b are levels (0..LOGIC_MAX); only their
// on/off matters here. returns LOGIC_MAX or 0.
uint8_t logic_gate_eval(logic_gate_kind kind, uint8_t a, uint8_t b);

// gather a gate cell's two input levels off the grid, honouring its facing.
// writes into *a and *b (either may be set to 0 if no feeder is present). for a
// NOT gate, *a holds the back-face input and *b is forced to 0.
void logic_gate_read_inputs(logic_grid *g, const logic_cell *c,
                            uint8_t *a, uint8_t *b);

// full evaluation: read inputs from the grid and apply the table. returns the
// output level the gate should drive this tick.
uint8_t logic_gate_compute(logic_grid *g, const logic_cell *c);

// name for the dump.
const char *logic_gate_name(logic_gate_kind kind);

#endif
