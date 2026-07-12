#include "lt_skirt.h"
#include "../../world/block.h"
#include "../../config.h"
static const int H_FACE[4]      = { LT_FACE_PX, LT_FACE_NX, LT_FACE_PZ, LT_FACE_NZ }
;
static const int H_OFF[4][3]    = { {1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1} }
;
static float side_shade(void) { return 0.72f; }

void lt_skirt_edge(lt_mesh *m, int face,
                   float bx, float by, float bz, float cell, float depth,
                   int tile, int light) {
    float l = (float)light / (float)MAX_LIGHT;
if (l > 1.0f) l = 1.0f;
l *= side_shade();
float tu = (float)(tile % ATLAS_TILES_X) / (float)ATLAS_TILES_X;
float tv = (float)(tile / ATLAS_TILES_X) / (float)ATLAS_TILES_Y;
float ts_u = 1.0f / (float)ATLAS_TILES_X;
float ts_v = 1.0f / (float)ATLAS_TILES_Y;
float x0 = bx, x1 = bx + cell;
float z0 = bz, z1 = bz + cell;
float yt = by;
float yb = by - depth;
lt_vertex a, b, c, d;
a.u = tu;
a.v = tv;
b.u = tu + ts_u;
b.v = tv;
c.u = tu + ts_u;
c.v = tv + ts_v;
d.u = tu;
d.v = tv + ts_v;
a.light = b.light = c.light = d.light = l;
switch (face) {
    case LT_FACE_PX:
        a.x=x1; a.y=yb; a.z=z0;  b.x=x1; b.y=yb; b.z=z1;
        c.x=x1; c.y=yt; c.z=z1;  d.x=x1; d.y=yt; d.z=z0;
        break;
    case LT_FACE_NX:
        a.x=x0; a.y=yb; a.z=z1;  b.x=x0; b.y=yb; b.z=z0;
        c.x=x0; c.y=yt; c.z=z0;  d.x=x0; d.y=yt; d.z=z1;
        break;
    case LT_FACE_PZ:
        a.x=x1; a.y=yb; a.z=z1;  b.x=x0; b.y=yb; b.z=z1;
        c.x=x0; c.y=yt; c.z=z1;  d.x=x1; d.y=yt; d.z=z1;
        break;
    default: /* LT_FACE_NZ */
        a.x=x0; a.y=yb; a.z=z0;  b.x=x1; b.y=yb; b.z=z0;
        c.x=x1; c.y=yt; c.z=z0;  d.x=x0; d.y=yt; d.z=z0;
        break;
    }

    lt_mesh_quad(m, a, b, c, d, 1);
}

int lt_skirt_build(lt_mesh *m, const lt_grid *g, const lt_source *src,
                   lt_seam_mask mask) {
    if (mask == 0) return 0;

    float cell  = (float)g->step;
    float depth = LT_SKIRT_DEPTH;
    int added;

    for (int s = 0; s < 4; s++) {
        int face = H_FACE[s];
        if (!lt_seam_has(mask, face)) continue;

        int ax = H_OFF[s][0];
        int az = H_OFF[s][2];

        for (int y = 0; y < g->ny; y++) {
            for (int z = 0; z < g->nz; z++) {
                for (int x = 0; x < g->nx; x++) {
                    if (!lt_grid_cell_solid(g, x, y, z)) continue;

                    // only border cells on this face — the ones whose outward
                    // neighbour would be in the next chunk.
                    int on_border =
                        (ax > 0 && x == g->nx - 1) ||
                        (ax < 0 && x == 0) ||
                        (az > 0 && z == g->nz - 1) ||
                        (az < 0 && z == 0);
                    if (!on_border) continue;

                    // and only if that face is actually exposed (top of a slope)
                    // — skirting buried border cells is wasted geometry.
                    if (lt_grid_cell_solid(g, x + ax, y, z + az)) continue;

                    block_id id = lt_grid_get(g, x, y, z);
                    int tile = block_face_tile(id, face);

                    float bx = (float)(src->base_x) + (float)x * cell;
                    float by = (float)y * cell;
                    float bz = (float)(src->base_z) + (float)z * cell;

                    int light = src->light
                        ? src->light(src->ctx, (int)bx, (int)by, (int)bz)
                        : MAX_LIGHT;

                    lt_skirt_edge(m, face, bx, by, bz, cell, depth, tile, light);
                    added++;
                }
            }
        }
    }
    return added;
}
