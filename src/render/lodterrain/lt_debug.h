#ifndef RENDER_LODTERRAIN_LT_DEBUG_H
#define RENDER_LODTERRAIN_LT_DEBUG_H

#include "lt_types.h"
#include "lt_grid.h"
#include "lt_mesh.h"
#include "../../math/vec3.h"

// debug + validation helpers for the lod terrain. these saved me more than once
// when a seam looked wrong — the validators catch a malformed mesh before it
// ever hits the gpu, and the band colour lets me tint each level so i can SEE
// where the rings fall. headless on purpose; no gl.

// sanity-check a built mesh: every index in range, index count a multiple of 3,
// no NaN coords. returns 1 if clean, 0 and fills `reason` (if non-NULL) with a
// short literal describing the first problem found.
int lt_debug_validate_mesh(const lt_mesh *m, const char **reason);

// check a grid is internally consistent: dims match the level's step, no cell
// holds a block id past BLOCK_COUNT. returns 1 if clean.
int lt_debug_validate_grid(const lt_grid *g);

// a distinct debug colour per lod level, for the "show lod" overlay mode. level
// 0 green (cheap/near), shading to red at the coarsest. wraps if you somehow
// pass a level past the count.
vec3 lt_debug_level_color(int level);

// count exposed faces a grid *would* emit, without building. handy for asserting
// the surface pass emitted exactly that many quads in a test.
int  lt_debug_expected_faces(const lt_grid *g);

#endif
