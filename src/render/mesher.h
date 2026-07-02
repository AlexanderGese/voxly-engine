#ifndef RENDER_MESHER_H
#define RENDER_MESHER_H

#include "mesh.h"
#include "../world/world.h"

// builds a mesh for a single chunk directly into the chunk's own vao/vbo.
// naive per-face culling. face culling skips faces against opaque neighbors.

void mesher_build_chunk(world *w, chunk *c);

#endif
