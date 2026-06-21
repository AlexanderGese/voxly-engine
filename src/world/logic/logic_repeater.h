#ifndef WORLD_LOGIC_REPEATER_H
#define WORLD_LOGIC_REPEATER_H

#include "logic_types.h"
#include "logic_grid.h"

// repeater state machine. a repeater reads the cell behind its output face,
// waits `delay` ticks, then latches that input to its output at full strength.
// it is one-directional (back -> front) and can be locked by a powered side
// input, which freezes its current output - that's the latch trick.
//
// the cell fields we lean on:
// facing  - output direction
// delay   - 1..4 ticks
// phase   - countdown of ticks remaining before the pending input commits
// power   - current output
// next    - staged output the countdown is heading toward
// flags   - LOGIC_CF_STAGED while a transition is in flight

// read the raw input level feeding a repeater (the back face). returns 0..MAX.
uint8_t logic_repeater_input(logic_grid *g, const logic_cell *c);

// is the repeater currently locked by a powered perpendicular neighbour? a
// locked repeater holds its output and ignores input changes.
int logic_repeater_locked(logic_grid *g, const logic_cell *c);

// advance a repeater by one tick. returns 1 if the cell's output changed (so
// the net knows to wake downstream). this drives the countdown and the latch.
int logic_repeater_tick(logic_grid *g, logic_cell *c);

// nudge a repeater because an input may have changed. (re)arms the countdown if
// the target output differs from where we're already heading. returns 1 if a
// new transition was armed.
int logic_repeater_poke(logic_grid *g, logic_cell *c);

// clamp/normalise a delay value into the legal notch range.
uint8_t logic_repeater_clamp_delay(int d);

#endif
