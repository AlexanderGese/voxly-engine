#ifndef WORLD_HEIGHTMAP_SAMPLE_H
#define WORLD_HEIGHTMAP_SAMPLE_H

#include "heightmap_params.h"

// raw field samples for one world column, before any spline is applied. each
// channel is in [-1,1]. this is the "what the noise thinks" stage; the spline
// stage downstream turns these into actual heights. kept separate so the f3
// overlay and the biome picker can read the same numbers we build terrain from.

typedef struct {
    float continentalness;   // continent vs ocean
    float erosion;           // jagged vs worn flat
    float peaks_valleys;     // ridged, the literal hills
    float weirdness;         // polarity flips for variety
} heightmap_fields;

// sample all four channels at a world column.
void heightmap_sample_fields(const heightmap_params *p, int wx, int wz,
                             heightmap_fields *out);

// the individual channels, in case a caller only needs one and doesnt want to
// pay for the other three. continentalness is the warped one.
float heightmap_sample_continental(const heightmap_params *p, int wx, int wz);
float heightmap_sample_erosion(const heightmap_params *p, int wx, int wz);
float heightmap_sample_peaks_valleys(const heightmap_params *p, int wx, int wz);
float heightmap_sample_weirdness(const heightmap_params *p, int wx, int wz);

#endif
