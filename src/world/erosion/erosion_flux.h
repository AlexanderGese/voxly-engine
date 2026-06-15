#ifndef WORLD_EROSION_FLUX_H
#define WORLD_EROSION_FLUX_H

#include "erosion_types.h"
#include "erosion_field.h"

// drainage / flow accumulation over the eroded heightmap. this is the dry
// analytic cousin of the droplet sim: instead of throwing thousands of marbles
// we compute, for every cell, how much upstream area drains through it (the
// d8 flow-accumulation trick from terrain analysis). the result tells us where
// the rivers *want* to be, which the biome pass uses to paint river/lake beds
// and the structure pass uses to avoid dropping a village in a flood plain.

// per-cell flow data. accum is in upstream cell count (1 = a ridge top that
// nothing drains into, big = a valley floor everything funnels through).
typedef struct {
    int   to[EROSION_CELLS];     // flat index of the downhill neighbour, -1 = pit
    float accum[EROSION_CELLS];  // upstream drainage area
    float wet[EROSION_CELLS];    // smoothed accum normalised to [0,1] for paint
} erosion_flux;

// compute the d8 downhill pointer field. each cell points at its steepest
// lower neighbour; cells with no lower neighbour are pits (-1).
void erosion_flux_d8(const erosion_field *f, erosion_flux *fx);

// accumulate drainage area by processing cells from high to low so a cell is
// always summed after everything that drains into it. must run after d8.
void erosion_flux_accumulate(const erosion_field *f, erosion_flux *fx);

// normalise accum into wet[] with a log curve (river width scales sub-linearly
// with drainage) and a light blur so banks arent single-cell ragged.
void erosion_flux_normalise(erosion_flux *fx);

// 1 if this interior cell carries enough drainage to be a river channel.
int erosion_flux_is_channel(const erosion_flux *fx, int x, int z, float thresh);

// convenience: d8 + accumulate + normalise in one call.
void erosion_flux_compute(const erosion_field *f, erosion_flux *fx);

#endif
