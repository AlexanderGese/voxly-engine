#ifndef WORLD_LOGIC_SIGNAL_H
#define WORLD_LOGIC_SIGNAL_H

#include "logic_types.h"

// the little numeric helpers that define what a "signal" actually does. all of
// it is saturating integer arithmetic on the 0..LOGIC_MAX scale. pulled into
// one file so the rules live in exactly one place and the gate/propagate code
// reads like prose.

// clamp an int into the legal power band.
uint8_t logic_clamp(int p);

// power after travelling one wire step. saturates at zero, never wraps.
uint8_t logic_attenuate(uint8_t p);

// power after travelling n wire steps.
uint8_t logic_attenuate_n(uint8_t p, int n);

// the stronger of two levels. wires take the max of all their feeds.
uint8_t logic_max2(uint8_t a, uint8_t b);

// boolean view of a level: anything above zero reads as "on".
int logic_is_on(uint8_t p);

// invert a boolean level: on -> 0, off -> LOGIC_MAX. used by NOT and torches.
uint8_t logic_invert(uint8_t p);

// fold a level down to a clean full/empty. gates restore signal strength, so
// after a gate the output is either dead or LOGIC_MAX, never something in
// between.
uint8_t logic_restore(uint8_t p);

#endif
