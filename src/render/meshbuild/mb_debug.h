#ifndef RENDER_MESHBUILD_MB_DEBUG_H
#define RENDER_MESHBUILD_MB_DEBUG_H

// sanity checks for built meshes. cheap invariants that caught real bugs while
// writing the greedy pass (degenerate quads, indices pointing past the vertex
// array, NaN positions from a bad scatter). compiled in always; the checks are
// O(indices) and only run when you ask.

#include "mb_types.h"

typedef struct {
    int ok;
    int degenerate_tris;   // triangles with zero area
    int oob_indices;       // indices >= vertex count
    int nan_verts;         // verts with a NaN/inf coord
    int bad_light;         // light outside [0,1]
} mb_check;

// validate a result. fills `out` with counts; out->ok is 1 only if everything
// is clean. doesnt modify the mesh.
mb_check meshbuild_check(const mb_result *r);

// one-line summary to the log at LOG_WARN if anything is off, LOG_DEBUG if
// clean. returns out->ok.
int meshbuild_check_log(const mb_result *r);

#endif
