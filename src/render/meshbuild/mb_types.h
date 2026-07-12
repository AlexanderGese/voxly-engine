#ifndef RENDER_MESHBUILD_MB_TYPES_H
#define RENDER_MESHBUILD_MB_TYPES_H
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
typedef struct {
    int          base_x, base_z;   // chunk origin in world block coords
    mb_sample_fn sample;
    mb_light_fn  light;
    void        *ctx;              // passed back to the callbacks
    int          merge;            // enable greedy merging
} mb_ctx;
#endif
