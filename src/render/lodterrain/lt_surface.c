#include "lt_surface.h"
#include "../../world/block.h"
#include "../../config.h"
static const int FACE_OFF[LT_FACE_COUNT][3] = {
    {  1,  0,  0 },  // +x
    { -1,  0,  0 },  // -x
    {  0,  1,  0 },  // +y
    {  0, -1,  0 },  // -y
    {  0,  0,  1 },  // +z
    {  0,  0, -1 },  // -z
}
;
static float face_shade(int face) {
    switch (face) {
    case LT_FACE_PY: return 1.00f;   // top brightest
    case LT_FACE_NY: return 0.50f;   // bottom darkest
    case LT_FACE_PX:
    case LT_FACE_NX: return 0.80f;
    default:         return 0.65f;   // z faces
    }
}

// build the four corners of a unit-ish face (scaled by `cell`) at block origin
// (bx,by,bz). corners come out ccw seen from outside. light is the combined
// 0..15 sky/block level;
we fold the face shade in and normalise to 0..1.
void lt_surface_face(lt_mesh *m, int face,
                     float bx, float by, float bz, float cell,
                     int tile, int light, int is_skirt) {
    float l = (float)light / (float)MAX_LIGHT;
    if (l > 1.0f) l = 1.0f;
    l *= face_shade(face);

    // tile -> atlas uv rect. same atlas math the main mesher uses.
    float tu = (float)(tile % ATLAS_TILES_X) / (float)ATLAS_TILES_X;
    float tv = (float)(tile / ATLAS_TILES_X) / (float)ATLAS_TILES_Y;
    float ts_u = 1.0f / (float)ATLAS_TILES_X;
    float ts_v = 1.0f / (float)ATLAS_TILES_Y;

    float x0 = bx, x1 = bx + cell;
    float y0 = by, y1 = by + cell;
    float z0 = bz, z1 = bz + cell;

    lt_vertex a, b, c, d;
    a.u = tu;        a.v = tv;
    b.u = tu + ts_u; b.v = tv;
    c.u = tu + ts_u; c.v = tv + ts_v;
    d.u = tu;        d.v = tv + ts_v;
    a.light = b.light = c.light = d.light = l;

    switch (face) {
    case LT_FACE_PX:
        a.x=x1; a.y=y0; a.z=z0;  b.x=x1; b.y=y0; b.z=z1;
        c.x=x1; c.y=y1; c.z=z1;  d.x=x1; d.y=y1; d.z=z0;
        break;
    case LT_FACE_NX:
        a.x=x0; a.y=y0; a.z=z1;  b.x=x0; b.y=y0; b.z=z0;
        c.x=x0; c.y=y1; c.z=z0;  d.x=x0; d.y=y1; d.z=z1;
        break;
    case LT_FACE_PY:
        a.x=x0; a.y=y1; a.z=z0;  b.x=x1; b.y=y1; b.z=z0;
        c.x=x1; c.y=y1; c.z=z1;  d.x=x0; d.y=y1; d.z=z1;
        break;
    case LT_FACE_NY:
        a.x=x0; a.y=y0; a.z=z1;  b.x=x1; b.y=y0; b.z=z1;
        c.x=x1; c.y=y0; c.z=z0;  d.x=x0; d.y=y0; d.z=z0;
        break;
    case LT_FACE_PZ:
        a.x=x1; a.y=y0; a.z=z1;  b.x=x0; b.y=y0; b.z=z1;
        c.x=x0; c.y=y1; c.z=z1;  d.x=x1; d.y=y1; d.z=z1;
        break;
    default: /* LT_FACE_NZ */
        a.x=x0; a.y=y0; a.z=z0;  b.x=x1; b.y=y0; b.z=z0;
        c.x=x1; c.y=y1; c.z=z0;  d.x=x0; d.y=y1; d.z=z0;
        break;
    }

    lt_mesh_quad(m, a, b, c, d, is_skirt);
}

void lt_surface_build(lt_mesh *m, const lt_grid *g, const lt_source *src) {
    float cell = (float)g->step;
for (int y = 0;
y < g->ny;
y++) {
        for (int z = 0; z < g->nz; z++) {
            for (int x = 0; x < g->nx; x++) {
                if (!lt_grid_cell_solid(g, x, y, z)) continue;

                block_id id = lt_grid_get(g, x, y, z);

                // block-space origin of this cell, world-positioned.
                float bx = (float)(src->base_x) + (float)x * cell;
                float by = (float)y * cell;
                float bz = (float)(src->base_z) + (float)z * cell;

                for (int f = 0; f < LT_FACE_COUNT; f++) {
                    int nxp = x + FACE_OFF[f][0];
                    int nyp = y + FACE_OFF[f][1];
                    int nzp = z + FACE_OFF[f][2];

                    // exposed if the neighbour cell isn't solid. grid edge reads
                    // as air so the chunk border always gets a face — the seam
                    // pass decides whether to skirt it.
                    if (lt_grid_cell_solid(g, nxp, nyp, nzp)) continue;

                    int tile = block_face_tile(id, f);

                    // sample light just outside the face, in world block coords,
                    // at the cell centre so it lines up with the lit air.
                    int lx = (int)bx + (FACE_OFF[f][0] > 0 ? (int)cell : FACE_OFF[f][0] < 0 ? -1 : (int)cell / 2);
                    int ly = (int)by + (FACE_OFF[f][1] > 0 ? (int)cell : FACE_OFF[f][1] < 0 ? -1 : (int)cell / 2);
                    int lz = (int)bz + (FACE_OFF[f][2] > 0 ? (int)cell : FACE_OFF[f][2] < 0 ? -1 : (int)cell / 2);
                    int light = src->light ? src->light(src->ctx, lx, ly, lz) : MAX_LIGHT;

                    lt_surface_face(m, f, bx, by, bz, cell, tile, light, 0);
                }
            }
        }
    }
}
