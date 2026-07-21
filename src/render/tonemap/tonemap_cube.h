#ifndef RENDER_TONEMAP_CUBE_H
#define RENDER_TONEMAP_CUBE_H

#include <stddef.h>

#include "tonemap_lut.h"

// loader for the adobe / resolve ".cube" 3d lut format. it's a plain text
// file: a few header keywords then dim^3 lines of "r g b" floats in rgb-major
// order, which is exactly how tonemap_lut stores them. comments start with #.
//
// we only handle LUT_3D — nobody ships 1d cubes for color grading and i'm not
// writing a parser for a thing i'll never load.

// parse a cube file from an in-memory text buffer into `out` (which gets a
// fresh allocation; any prior contents are destroyed). returns 1 on success.
// on any malformed line it logs and bails, leaving `out` as a valid identity
// so the rest of the pipeline keeps working.
int tonemap_cube_parse(tonemap_lut *out, const char *text, size_t len);

// convenience: read a cube file off disk and parse it. returns 1 on success.
int tonemap_cube_load_file(tonemap_lut *out, const char *path);

#endif
