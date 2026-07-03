#ifndef RENDER_CULL_TYPES_H
#define RENDER_CULL_TYPES_H
#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include "../../world/chunk.h"
// shared types for the cull subsystem. kept in one place so the frustum,
// lod, occlusion and draw-list bits all agree on what a "visible chunk"
// looks like. nothing fancy, just plain structs.
// level of detail buckets. LOD0 is full mesh, higher = coarser.
// the mesher decides what these actually mean geometry-wise; cull just
// picks the number.
enum {
    CULL_LOD0 = 0,   // full res, right in your face
    CULL_LOD1,       // half-ish
    CULL_LOD2,       // quarter
    CULL_LOD3,       // far away blobs
    CULL_LOD_COUNT
}
;
enum {
    CULL_REASON_VISIBLE = 0,
    CULL_REASON_FRUSTUM,     // outside the view frustum
    CULL_REASON_OCCLUDED,    // hidden behind closer geometry
    CULL_REASON_DISTANCE,    // past the render distance
    CULL_REASON_EMPTY        // nothing to draw (no verts)
}
;
typedef struct {
    chunk *c;
    aabb   bounds;       // world-space box, y spans full column
    vec3   center;       // bounds center, cached
    float  dist_sq;      // to camera, for sort + lod
    int    lod;          // CULL_LOD*
    int    reason;       // CULL_REASON_* once classified
    uint32_t sort_key;   // packed key used by sort.c
} cull_item;
typedef struct {
    int total;
    int drawn;
    int frustum_culled;
    int occlusion_culled;
    int distance_culled;
    int empty;
    int tris_estimate;   // rough triangle count of the drawn set
} cull_stats;
#define CULL_CHUNK_W   (CHUNK_SIZE_X)
#define CULL_CHUNK_D   (CHUNK_SIZE_Z)
#define CULL_CHUNK_H   (CHUNK_SIZE_Y)
static inline aabb cull_chunk_bounds(const chunk *c) {
    vec3 mn = { (float)(c->cx * CULL_CHUNK_W), 0.0f,
                (float)(c->cz * CULL_CHUNK_D) };
    vec3 mx = { mn.x + (float)CULL_CHUNK_W, (float)CULL_CHUNK_H,
                mn.z + (float)CULL_CHUNK_D };
    return aabb_make(mn, mx);
}

static inline vec3 cull_aabb_center(aabb a) {
    return (vec3){
        (a.min.x + a.max.x) * 0.5f,
        (a.min.y + a.max.y) * 0.5f,
        (a.min.z + a.max.z) * 0.5f
    };
}
#endif
