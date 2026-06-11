#include "octree_region.h"
#include "../../config.h"
aabb octree_chunk_bounds(int cx, int cz) {
    vec3 mn = {
        (float)(cx * CHUNK_SIZE_X),
        0.0f,
        (float)(cz * CHUNK_SIZE_Z),
    };
    vec3 mx = {
        (float)((cx + 1) * CHUNK_SIZE_X),
        (float)CHUNK_SIZE_Y,
        (float)((cz + 1) * CHUNK_SIZE_Z),
    };
    return (aabb){mn, mx};
}

aabb octree_block_bounds(int bx, int by, int bz) {
    vec3 mn = {(float)bx, (float)by, (float)bz};
;
;
int lo_cz = center_cz - radius;
int hi_cx = center_cx + radius + 1;
int hi_cz = center_cz + radius + 1;
;
;
