#ifndef WORLD_OREGEN_SCATTER_H
#define WORLD_OREGEN_SCATTER_H

#include <stdint.h>

// horizontal scatter helpers. plain rng picks for vein columns tend to
// clump two veins on top of each other now and then; this gives a jittered
// stratified scatter instead, so a chunk's veins spread out more evenly
// without going full poisson-disk (which is overkill at 16x16).
//
// the idea: chop the chunk footprint into a small grid of cells and drop at
// most one point per cell, jittered inside it. deterministic on the cell
// coords + seed so reruns match.

// one scattered point in local chunk coords (0..15).
typedef struct {
    int lx, lz;
} oregen_point;

// fill out[] with up to want stratified points in a w x d footprint.
// returns how many were written (<= want and <= out_cap). the grid is sized
// to roughly bracket want, so asking for more spreads them finer.
int oregen_scatter_grid(oregen_point *out, int out_cap, int want,
                        int w, int d, uint32_t seed);

// minimum chebyshev spacing actually achieved between the points, handy as
// a quality check / for tuning. returns large if <2 points.
int oregen_scatter_min_spacing(const oregen_point *pts, int n);

#endif
