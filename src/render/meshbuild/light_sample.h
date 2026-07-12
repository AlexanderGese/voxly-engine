#ifndef RENDER_MESHBUILD_LIGHT_SAMPLE_H
#define RENDER_MESHBUILD_LIGHT_SAMPLE_H

// face lighting. the naive mesher just samples the single cell in front of the
// face. that flickers along greedy quad edges, so here we smooth: each corner
// averages the 4 light values touching it (same neighbourhood as AO), and a
// greedy quad only merges when its smoothed corners match the running run.

#include "mb_types.h"

// raw light at the cell in front of a face, clamped to MB_MIN_FACE_LIGHT so we
// never emit pure black. this is the value used for merge-equality tests
// (smoothing happens at pack time on a per-corner basis).
int mb_light_face(const mb_ctx *c, int x, int y, int z, int face);

// smoothed light for the four corners of a face, 0..15 each, written to out.
// uses the same 8-neighbour ring as AO so light and shadow line up.
void mb_light_quad(const mb_ctx *c, int x, int y, int z, int face, int out[4]);

#endif
