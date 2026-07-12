#ifndef RENDER_MESHBUILD_AO_SAMPLER_H
#define RENDER_MESHBUILD_AO_SAMPLER_H

// per-vertex ambient occlusion, the 0fps.net flavour. for each of the 4
// corners of a face we look at the two edge-adjacent blocks and the one
// diagonal block sitting in front of the face. more of them solid => darker.
//
// the engine already has a render/ambient_occlusion.c but that one bakes in
// the world pointer and only really handles top/bottom. this version is
// axis-generic (greedy meshing needs all 6 faces correct) and works off the
// mb_ctx sample callback so it can run off-thread.

#include "mb_types.h"

// the classic vertex AO term, 0 (darkest) .. 3 (open). `s1`/`s2` are the two
// side neighbours, `c` the corner. if both sides are solid the corner is fully
// occluded regardless of the diagonal.
int mb_ao_corner(int s1, int s2, int c);

// fill ao[0..3] for one face at chunk-local cell (x,y,z). corners are returned
// in the canonical winding order used by the packer: starting at the (u0,v0)
// corner and going ccw around the face when viewed from outside.
void mb_ao_quad(const mb_ctx *c, int x, int y, int z, int face, int ao[4]);

// AO can break a quad's bilinear interpolation along the wrong diagonal,
// producing a visible crease. when the two opposite-corner sums disagree we
// flip the triangulation. returns 1 if the quad should be split along the
// (1,3) diagonal instead of the default (0,2).
int mb_ao_flip(const int ao[4]);

#endif
