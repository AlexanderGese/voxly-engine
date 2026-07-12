#ifndef RENDER_LODTERRAIN_LT_TYPES_H
#define RENDER_LODTERRAIN_LT_TYPES_H

// shared types for the terrain lod module. nothing in here touches GL — the
// builder spits out cpu-side arrays and lt_gpu uploads them. keeps the build
// path testable without a window, same trick meshbuild plays.

#include "lt_config.h"
#include "../../world/block.h"
#include <stdint.h>

// the six face directions, matching the engine's convention everywhere else:
// 0=+x, 1=-x, 2=+y(top), 3=-y(bot), 4=+z, 5=-z.
enum {
    LT_FACE_PX = 0,
    LT_FACE_NX,
    LT_FACE_PY,
    LT_FACE_NY,
    LT_FACE_PZ,
    LT_FACE_NZ,
    LT_FACE_COUNT
};

// neighbour-level bitmask. for seam stitching a chunk needs to know which of
// its four horizontal neighbours are coarser than it. one bit per horizontal
// face (px, nx, pz, nz). a set bit means "that neighbour is at a higher lod
// level number than me", i.e. coarser, i.e. i must stitch toward it.
typedef uint8_t lt_seam_mask;

#define LT_SEAM_PX  (1u << 0)
#define LT_SEAM_NX  (1u << 1)
#define LT_SEAM_PZ  (1u << 2)
#define LT_SEAM_NZ  (1u << 3)

// a downsampled voxel grid for one chunk at one lod level. cells are the
// dominant (most common opaque) block in each step^3 box, or air if the box
// failed the solid threshold. dimensions are the full-res dims divided by step.
typedef struct {
    int      level;          // which lod this grid was sampled at
    int      step;           // 1<<level
    int      nx, ny, nz;     // cell counts on each axis
    block_id *cells;         // nx*ny*nz, dominant block or BLOCK_AIR
} lt_grid;

// cpu-side vertex. byte-compatible with render/mesh.h `vertex` so lt_gpu can
// hand it straight to the same shader. ao is folded into light like meshbuild.
typedef struct {
    float x, y, z;
    float u, v;
    float light;
} lt_vertex;

// a build result: indexed triangle soup. verts/indices are darray-allocated and
// owned by the caller (freed via lt_mesh_free). quad_count is informational.
typedef struct {
    lt_vertex *verts;        // darray
    uint32_t  *indices;      // darray
    int        quad_count;
    int        skirt_quads;  // how many of those were seam skirts
} lt_mesh;

// neighbour/light sampling callbacks. the builder never reaches into world
// directly so a flattened snapshot can be swapped in for a worker thread.
typedef block_id (*lt_sample_fn)(void *ctx, int wx, int wy, int wz);
typedef int      (*lt_light_fn) (void *ctx, int wx, int wy, int wz);

// everything a build needs about where it is and how to read the world.
typedef struct {
    int          base_x, base_z;   // chunk origin in world block coords
    lt_sample_fn sample;
    lt_light_fn  light;
    void        *ctx;
} lt_source;

#endif
