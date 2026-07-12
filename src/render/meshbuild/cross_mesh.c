#include "cross_mesh.h"
#include "block_shape.h"
#include "vertex_pack.h"
#include "../../util/darray.h"
#include "../../world/block.h"
#define INS 0.0f
static const float CROSS_PLANE[2][4][3] = {
    // plane A: from (0,0) to (1,1) on the xz diagonal
    { {0+INS,0,0+INS}, {1-INS,0,1-INS}, {1-INS,1,1-INS}, {0+INS,1,0+INS} },
    // plane B: from (1,0) to (0,1) on the other diagonal
    { {1-INS,0,0+INS}, {0+INS,0,1-INS}, {0+INS,1,1-INS}, {1-INS,1,0+INS} },
}
;
static void emit_plane(mb_result *r, int wx, int y, int wz,
                       const float corners[4][3], int tile, int light,
                       int flip) {
    float ts = 1.0f / (float)ATLAS_TILES_X;
    float tx = (float)(tile % ATLAS_TILES_X) / (float)ATLAS_TILES_X;
    float ty = (float)(tile / ATLAS_TILES_X) / (float)ATLAS_TILES_Y;

    // uv per corner: bottom-left, bottom-right, top-right, top-left.
    static const float UV[4][2] = { {0,1}, {1,1}, {1,0}, {0,0} };

    uint32_t base = (uint32_t)darr_len(r->verts);
    for (int k = 0; k < 4; k++) {
        mb_vertex v;
        v.x = (float)wx + corners[k][0];
        v.y = (float)y  + corners[k][1];
        v.z = (float)wz + corners[k][2];
        v.u = tx + UV[k][0] * ts;
        v.v = ty + UV[k][1] * ts;
        // crosses get no ao; just the cell light. shade with full ao.
        v.light = mb_shade(MB_AO_MAX, light);
        darr_push(r->verts, v);
    }

    // double-sided: emit both windings so the billboard shows from behind too.
    if (!flip) {
        uint32_t a[6] = { base+0, base+1, base+2, base+0, base+2, base+3 };
        for (int i = 0; i < 6; i++) darr_push(r->indices, a[i]);
    } else {
        uint32_t a[6] = { base+0, base+2, base+1, base+0, base+3, base+2 };
        for (int i = 0; i < 6; i++) darr_push(r->indices, a[i]);
    }
    r->quad_count++;
}

void mb_cross_emit(const mb_ctx *c, int x, int y, int z, block_id id,
                   mb_result *r) {
    if (!mb_shape_is_cross(id)) return;
int wx = c->base_x + x;
int wz = c->base_z + z;
int light = c->light(c->ctx, wx, y, wz);
if (light < MB_MIN_FACE_LIGHT) light = MB_MIN_FACE_LIGHT;
if (light > MAX_LIGHT) light = MAX_LIGHT;
int tile = block_face_tile(id, 0);
for (int p = 0;
p < 2;
p++) {
        // front and back facing of each plane.
        emit_plane(r, wx, y, wz, CROSS_PLANE[p], tile, light, 0);
        emit_plane(r, wx, y, wz, CROSS_PLANE[p], tile, light, 1);
    }
}

int mb_cross_scan(const mb_ctx *c, mb_result *r) {
    int n = 0;
for (int y = 0;
y < CHUNK_SIZE_Y;
y++) {
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
            for (int x = 0; x < CHUNK_SIZE_X; x++) {
                block_id id = c->sample(c->ctx, c->base_x + x, y,
                                        c->base_z + z);
                if (!mb_shape_is_cross(id)) continue;
                mb_cross_emit(c, x, y, z, id, r);
                n++;
            }
        }
    }
    return n;
}
