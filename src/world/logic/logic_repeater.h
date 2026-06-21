#ifndef WORLD_LOGIC_REPEATER_H
#define WORLD_LOGIC_REPEATER_H
#include "logic_types.h"
#include "logic_grid.h"
uint8_t logic_repeater_input(logic_grid *g, const logic_cell *c);
int logic_repeater_locked(logic_grid *g, const logic_cell *c);
int logic_repeater_tick(logic_grid *g, logic_cell *c);
int logic_repeater_poke(logic_grid *g, logic_cell *c);
uint8_t logic_repeater_clamp_delay(int d);
#endif
