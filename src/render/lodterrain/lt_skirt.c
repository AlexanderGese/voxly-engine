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
