#ifndef RENDER_INSTANCING_PREFAB_H
#define RENDER_INSTANCING_PREFAB_H

// canned base-mesh builders. instancing is most useful for the same little
// shape stamped a thousand times — crates, torches, pebbles, grass tufts — so
// having one-liners to generate those shapes keeps callers honest. all of
// these emit into a caller-supplied darray of instancing_base_vertex (stb
// header-in-front array; pass the address of a NULL-initialised pointer).
//
// uvs are atlas tile ids resolved to the [0,1] square for a given tile, using
// the ATLAS_TILES_X/Y config constants. light is left at 1.0; the per-instance
// light scalar modulates it at draw time.

#include "instance_mesh.h"

// append a unit cube centered at the origin, spanning [-0.5, 0.5] on each
// axis. all six faces share `tile`. 36 verts (two tris per face, no indexing —
// this engine's meshes are non-indexed). returns the new vertex count of `out`.
int instancing_prefab_cube(instancing_base_vertex **out, int tile);

// append a "cross" / billboard-x: two intersecting vertical quads forming an X
// when viewed from above. the classic foliage/flower shape. spans [-0.5, 0.5]
// horizontally and [0, 1] vertically (rooted at y=0). 12 verts.
int instancing_prefab_cross(instancing_base_vertex **out, int tile);

// append a flat ground quad on the xz plane at y=0, spanning [-0.5, 0.5].
// useful for decals / puddles done as instances. 6 verts.
int instancing_prefab_quad(instancing_base_vertex **out, int tile);

// model-space bounds for each prefab, so the caller can pass a matching cull
// box to instancing_mesh_init / instancing_register_mesh.
aabb instancing_prefab_cube_box(void);
aabb instancing_prefab_cross_box(void);
aabb instancing_prefab_quad_box(void);

#endif
