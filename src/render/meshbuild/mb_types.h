#ifndef RENDER_MESHBUILD_MB_TYPES_H
#define RENDER_MESHBUILD_MB_TYPES_H

// shared types for the mesh builder. nothing in here touches GL — the builder
// produces plain cpu-side arrays and the renderer uploads them. keeps this
// module testable without a window.

#include "mb_config.h"
#include "../../world/block.h"
#include <stdint.h>

// a single greedy quad before it gets split into triangles. coords are in
// chunk-local block space. (u0,v0)-(u1,v1) is the merged extent on the slice
// plane, in block units, so a 1x1 face is (0,0)-(1,1).
//
// ao holds the four corner occlusion levels (0..3), one per quad corner, in
// the winding order the packer expects. tile is the atlas tile id. light is
// the averaged face light 0..15. face is the direction 0..5.
typedef struct {
    uint8_t face;
    uint8_t tile;
    uint8_t light;
    uint8_t ao[4];
    float   x, y, z;     // origin corner (min corner of the quad)
    float   du, dv;      // size along the two in-plane axes
} mb_quad;

// the per-vertex format we hand to the renderer. this MUST stay byte-compatible
// with render/mesh.h `vertex` (x,y,z,u,v,light) — meshbuild_emit just fills one
// of these per corner. spelled out separately so the builder doesnt drag in
// gl.h through mesh.h.
typedef struct {
    float x, y, z;
    float u, v;
    float light;         // ao already folded in
} mb_vertex;

// output of a build. caller owns the buffers and frees them with
// meshbuild_result_free. verts/indices are darray-allocated.
typedef struct {
    mb_vertex *verts;     // darray
    uint32_t  *indices;   // darray
    int        quad_count;
    int        merged_away;   // how many faces greedy merging removed
} mb_result;

// neighbour sampling callback signature. the builder never reaches into the
// world directly for out-of-chunk reads; it goes through this so we can swap
// in a flattened snapshot for the worker thread later. returns a block id.
typedef block_id (*mb_sample_fn)(void *ctx, int x, int y, int z);

// light sampling callback. returns combined light 0..15 at a world cell.
typedef int (*mb_light_fn)(void *ctx, int x, int y, int z);

// everything the builder needs about where it is and how to read neighbours.
typedef struct {
    int          base_x, base_z;   // chunk origin in world block coords
    mb_sample_fn sample;
    mb_light_fn  light;
    void        *ctx;              // passed back to the callbacks
    int          merge;            // enable greedy merging
} mb_ctx;

#endif
