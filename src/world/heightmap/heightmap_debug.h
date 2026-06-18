#ifndef WORLD_HEIGHTMAP_DEBUG_H
#define WORLD_HEIGHTMAP_DEBUG_H

#include "heightmap_params.h"
#include "heightmap_column.h"

// the f3 overlay support for the spline terrain. dumps the field breakdown for
// the column under the player so you can see why a hill is where it is. also a
// tiny ascii profiler that prints a cross-section of the terrain to stdout,
// which is how i actually tuned the curves. no render deps, just sprintf.

// one line, "cont +0.42 eros -0.11 pv +0.63 -> y 118 grass". buf must hold at
// least 96 chars. returns the length written.
int heightmap_debug_line(const heightmap_params *p, const heightmap_column *col,
                         char *buf, int cap);

// classify a column into a one-char terrain glyph for the ascii dump.
char heightmap_debug_glyph(const heightmap_params *p,
                           const heightmap_column *col);

// print an ascii east-west cross section of length `width` blocks starting at
// (wx,wz), one row of glyphs plus a height-scaled profile. dumps to stdout.
// purely a tuning toy, nothing in the engine proper calls it.
void heightmap_debug_profile(const heightmap_params *p, int wx, int wz,
                             int width);

// sanity check the spline tables: monotone-loc, in range, no NaNs. returns 0
// if everything looks sane, else a nonzero error code. cheap to run at boot.
int heightmap_debug_validate_curves(void);

#endif
