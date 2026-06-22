#ifndef WORLD_OREGEN_SCATTER_H
#define WORLD_OREGEN_SCATTER_H
#include <stdint.h>
// horizontal scatter helpers. plain rng picks for vein columns tend to
// clump two veins on top of each other now and then; this gives a jittered
// stratified scatter instead, so a chunk's veins spread out more evenly
// without going full poisson-disk (which is overkill at 16x16).
typedef struct {
    int lx, lz;
} oregen_point;
int oregen_scatter_grid(oregen_point *out, int out_cap, int want,
                        int w, int d, uint32_t seed);
int oregen_scatter_min_spacing(const oregen_point *pts, int n);
#endif
