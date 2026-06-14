#ifndef WORLD_CAVEGEN_DEBUG_H
#define WORLD_CAVEGEN_DEBUG_H
#include "cavegen_grid.h"
#include "cavegen_types.h"
// dev-only helpers. none of this runs in a real worldgen pass, its just what i
// reach for when a cave looks wrong and i need to actually see the grid instead
// of guessing from inside the game.
// dump one horizontal y-slice of the grid to stderr as ascii. '#'=solid,
// ' '=air, '~'=flooded, 'w'=worm, 'x'=sealed. one char per cell.
void cavegen_debug_slice_xz(const cavegen_grid *g, int y);
// dump a vertical x-slice (the z/y plane at a fixed x). good for eyeballing
// whether the surface crust held and how deep the caves go.
void cavegen_debug_slice_zy(const cavegen_grid *g, int x);
// percentage of eligible volume that ended up open. caves want maybe 8-18%;
// if youre seeing 40% something is very wrong and youre about to fall through
// the world.
float cavegen_debug_open_ratio(const cavegen_grid *g);
// sanity checks on a finished grid. returns 0 if all good, else a count of
// problems found (and logs each). mostly catches the crust getting breached.
int cavegen_debug_validate(const cavegen_grid *g, const cavegen_params *p);
#endif
