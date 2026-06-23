#ifndef WORLD_RAVINE_PATH_H
#define WORLD_RAVINE_PATH_H

#include <stdint.h>
#include "ravine_types.h"

// the canyon centreline. a chain of control knots laid across the world in
// world-block space (so a ravine that spans several chunks stays one continuous
// curve), sampled with a catmull-rom spline so the cut is smooth instead of a
// polyline of visible kinks. each knot carries a local half-width so the canyon
// can pinch and flare, plus a floor height so the channel can roll downhill
// along its length. knots are placed by walking a heading that wanders inside
// knot_jitter with a steady drift bias — that's what makes a ravine arc
// gracefully rather than zigzag.

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

// build a path for a ravine. the anchor (ax,az) is the world-space entry point
// the spline starts from. floor_top is the local surface height near the anchor
// — the floor descends from there as the spline runs. stream is the canyon's
// private rng seed so every chunk that touches it rebuilds the identical curve.
void ravine_path_build(ravine_path *path, const ravine_params *p,
                       float ax, float az, int floor_top, uint32_t stream);

// sample the spline at parameter t in [0,1]. writes interpolated centre x/z,
// half-width and floor. clamps t.
void ravine_path_sample(const ravine_path *path, float t,
                        float *out_x, float *out_z,
                        float *out_hw, float *out_floor);

// nearest-point query: for a world column (wx,wz) find the perpendicular
// distance to the centreline, plus the local half-width and floor there. returns
// 0 if the path has no usable segment. this is the heart of the falloff mask.
int  ravine_path_nearest(const ravine_path *path, float wx, float wz,
                         float *out_dist, float *out_hw, float *out_floor);

// cheap aabb reject in world space: does this box come within reach of the path?
int  ravine_path_touches(const ravine_path *path,
                         float min_x, float min_z, float max_x, float max_z);

#endif
