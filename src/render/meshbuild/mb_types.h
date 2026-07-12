#ifndef RENDER_MESHBUILD_MB_TYPES_H
#define RENDER_MESHBUILD_MB_TYPES_H
// shared types for the mesh builder. nothing in here touches GL — the builder
// produces plain cpu-side arrays and the renderer uploads them. keeps this
#include "mb_config.h"
#include "../../world/block.h"
#include <stdint.h>
typedef struct {
    uint8_t face;
    uint8_t tile;
    uint8_t light;
    uint8_t ao[4];
    float   x, y, z;     // origin corner (min corner of the quad)
    float   du, dv;      // size along the two in-plane axes
} mb_quad;
typedef struct {
    float x, y, z;
    float u, v;
    float light;         // ao already folded in
} mb_vertex;
typedef struct {
    mb_vertex *verts;     // darray
    uint32_t  *indices;   // darray
    int        quad_count;
    int        merged_away;   // how many faces greedy merging removed
} mb_result;
typedef block_id (*mb_sample_fn)(void *ctx, int x, int y, int z);
typedef int (*mb_light_fn)(void *ctx, int x, int y, int z);
typedef struct {
    int          base_x, base_z;   // chunk origin in world block coords
    mb_sample_fn sample;
    mb_light_fn  light;
    void        *ctx;              // passed back to the callbacks
    int          merge;            // enable greedy merging
} mb_ctx;
#endif
