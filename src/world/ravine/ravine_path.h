#ifndef WORLD_RAVINE_PATH_H
#define WORLD_RAVINE_PATH_H
#include <stdint.h>
#include "ravine_types.h"
typedef struct {
    float  x, z;        // knot position in world-block space
    float  half_width;  // local half-width of the floor here
    float  floor_y;     // world y of the canyon floor at this knot
} ravine_knot;
typedef struct {
    ravine_knot knots[RAVINE_MAX_KNOTS];
    int         count;

    // world-space bounds of the knots plus the widest reach, cached so the mask
    // pass can reject a far column without a full nearest-point search.
    float       min_x, max_x;
    float       min_z, max_z;
    float       reach;
} ravine_path;
#endif
