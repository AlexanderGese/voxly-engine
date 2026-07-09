#ifndef RENDER_GBUFFER_GEOMETRY_H
#define RENDER_GBUFFER_GEOMETRY_H

#include "gbuffer.h"
#include "../texture.h"

// the geometry pass. binds the mrt, clears it, sets up the geometry program
// and the block atlas, then hands the caller a draw scope where they issue
// their chunk draw calls. we don't know about chunks here directly — the
// renderer already has its world-visit machinery — so geometry_begin just
// gets everything into the right state and geometry_end restores it.

// begin writing geometry into the g-buffer. clears all attachments to the
// "empty" texel (albedo 0, normal +z, material default) and uploads camera
// matrices to prog_geometry. atlas is bound to texture unit 0.
void gbuffer_geometry_begin(gbuffer *g, const texture *atlas);

// per-draw helper: push a model matrix for the chunk about to be drawn.
// chunks are axis-aligned so this is usually just a translate, but the pass
// stays general.
void gbuffer_geometry_set_model(gbuffer *g, mat4 model);

// finish the geometry pass and unbind the mrt.
void gbuffer_geometry_end(gbuffer *g);

#endif
