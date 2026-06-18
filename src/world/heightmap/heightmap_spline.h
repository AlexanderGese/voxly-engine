#ifndef WORLD_HEIGHTMAP_SPLINE_H
#define WORLD_HEIGHTMAP_SPLINE_H

#include <stdint.h>

// a 1d control-point spline, the mojang-style terrain spline. you feed it a
// noise value in [-1,1] (continentalness, erosion, whatever) and it spits out
// a height-ish factor. the trick the whole subsystem leans on: terrain shape
// is a small handful of these tables, not a wall of if/else. swap a table,
// swap a planet.
//
// segments interpolate with a smoothstep by default, or hermite when the
// control points carry slopes (set HEIGHTMAP_SPLINE_HAS_SLOPE). points must
// be sorted by .loc ascending; the builder asserts on that in debug.

#define HEIGHTMAP_SPLINE_MAX_PTS  12

typedef struct {
    float loc;        // input location on [-1,1], the x of the knot
    float val;        // output value at this knot
    float slope;      // dy/dx at the knot, used only by hermite eval
} heightmap_spline_pt;

typedef struct {
    heightmap_spline_pt pts[HEIGHTMAP_SPLINE_MAX_PTS];
    int   n;
    int   hermite;    // 0 = smoothstep blend, 1 = cubic hermite with slopes
} heightmap_spline;

// start an empty spline. pick hermite up front, it changes how eval reads pts.
void  heightmap_spline_init(heightmap_spline *s, int hermite);

// append a knot. keeps them sorted-by-loc; returns 0 if the table is full.
int   heightmap_spline_add(heightmap_spline *s, float loc, float val, float slope);

// evaluate at t. clamps to the end knots outside the table range, no
// extrapolation. flat tails are a feature here, the ocean floor shouldnt
// keep dropping past the last control point.
float heightmap_spline_eval(const heightmap_spline *s, float t);

// derivative of the spline at t. handy for slope-aware blends and for the
// erosion pass that wants to know how steep the spline thinks a column is.
float heightmap_spline_deriv(const heightmap_spline *s, float t);

#endif
