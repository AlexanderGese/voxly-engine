#ifndef RENDER_MESHBUILD_VERTEX_PACK_H
#define RENDER_MESHBUILD_VERTEX_PACK_H

// turns an mb_quad into 4 mb_vertex + 6 indices and appends them to the result
// darrays. this is where chunk-local coords become world coords, where the
// atlas tile becomes uv rectangles, and where ao+light collapse into the single
// `light` float the shader reads.

#include "mb_types.h"

// fold a 0..3 ao level and a 0..15 light value into the final 0..1 brightness.
// ao is applied multiplicatively so a fully-occluded corner in bright light
// still darkens, which is the whole point.
float mb_shade(int ao, int light);

// emit one quad. base_x/base_z offset the chunk into world space. appends to
// r->verts and r->indices (both darrays, grown in place). handles the ao-driven
// triangle flip so the diagonal seam falls on the darker pair of corners.
void mb_pack_quad(mb_result *r, const mb_quad *q, int base_x, int base_z);

#endif
