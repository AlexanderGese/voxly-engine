#ifndef WORLD_LOGIC_GATE_H
#define WORLD_LOGIC_GATE_H
#include "logic_types.h"
#include "logic_grid.h"
uint8_t logic_gate_eval(logic_gate_kind kind, uint8_t a, uint8_t b);
void logic_gate_read_inputs(logic_grid *g, const logic_cell *c,
                            uint8_t *a, uint8_t *b);
#endif
