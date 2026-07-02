#ifndef RENDER_AMBIENT_OCCLUSION_H
#define RENDER_AMBIENT_OCCLUSION_H

#include "../world/world.h"

// per-vertex ambient occlusion for block faces. darkens corners and edges
// where blocks meet, giving a softer look without real shadows.
//
// the technique: for each vertex of a face, sample the 3 neighboring blocks
// in the corner direction. the AO factor is 0 (fully occluded) to 3 (open).
// output is a float 0..1 multiplied into the vertex light.

float ao_factor(world *w, int wx, int wy, int wz,
                int face, int vertex_index);

// convenience: compute AO for all 4 corners of a face at once.
// out[0..3] = AO factors for each corner vertex.
void ao_face(world *w, int wx, int wy, int wz, int face, float out[4]);

#endif
