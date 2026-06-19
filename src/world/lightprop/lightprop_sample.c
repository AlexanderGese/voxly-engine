#include "lightprop_sample.h"
#include "lightprop_access.h"

// the two in-plane axes for each face. face order matches block_face_tile:
// 0=+x 1=-x 2=+y 3=-y 4=+z 5=-z. for each face we store the face normal and the
// two tangent directions; the 4 corners are normal-cell + the four
// tangent-sign combos sampled diagonally. standard smooth-lighting setup.
typedef struct { int nx, ny, nz; int ux, uy, uz; int vx, vy, vz; } face_basis;

static const face_basis FACES[6] = {
    /* +x */ { 1, 0, 0,   0, 1, 0,   0, 0, 1 },
    /* -x */ {-1, 0, 0,   0, 1, 0,   0, 0, 1 },
    /* +y */ { 0, 1, 0,   1, 0, 0,   0, 0, 1 },
    /* -y */ { 0,-1, 0,   1, 0, 0,   0, 0, 1 },
    /* +z */ { 0, 0, 1,   1, 0, 0,   0, 1, 0 },
    /* -z */ { 0, 0,-1,   1, 0, 0,   0, 1, 0 },
};

// corner sign table: (us, vs) in {-1,+1} for the 4 corners, ccw.
static const int CORNER_U[4] = { -1,  1,  1, -1 };
static const int CORNER_V[4] = { -1, -1,  1,  1 };

static float cell_value(world *w, int x, int y, int z, float daylight) {
    if (!lp_y_in_range(y)) return 0.0f;
    block_id id = lp_get_block(w, x, y, z);
    if (!lp_transmits(id)) return 0.0f;   // can't see light inside a wall
    uint8_t b = lp_get_light(w, LP_BLOCK, x, y, z);
    uint8_t s = lp_get_light(w, LP_SKY, x, y, z);
    float fb = (float)b / (float)MAX_LIGHT;
    float fs = (float)s / (float)MAX_LIGHT * daylight;
    float v = fb > fs ? fb : fs;
    // gentle gamma so dim light isn't pitch black. matches the curve render/ uses.
    return v * v * (3.0f - 2.0f * v);
}

float lp_sample_cell(world *w, int wx, int wy, int wz, float daylight) {
    return cell_value(w, wx, wy, wz, daylight);
}

float lp_sample_vertex(world *w, int wx, int wy, int wz,
                       int face, int corner, float daylight) {
    if (face < 0 || face > 5) return 0.0f;
    if (corner < 0 || corner > 3) corner = 0;
    const face_basis *f = &FACES[face];
    int us = CORNER_U[corner], vs = CORNER_V[corner];

    // the cell sitting just outside the face (where light actually lives).
    int ox = wx + f->nx, oy = wy + f->ny, oz = wz + f->nz;

    // four cells around the vertex: the outside cell plus its u/v/uv diagonals.
    int s = 0; float acc = 0.0f;
    int offs[4][3] = {
        { 0, 0, 0 },
        { us*f->ux, us*f->uy, us*f->uz },
        { vs*f->vx, vs*f->vy, vs*f->vz },
        { us*f->ux + vs*f->vx, us*f->uy + vs*f->vy, us*f->uz + vs*f->vz },
    };
    for (int i = 0; i < 4; i++) {
        int cx = ox + offs[i][0], cy = oy + offs[i][1], cz = oz + offs[i][2];
        block_id id = lp_get_block(w, cx, cy, cz);
        if (!lp_transmits(id)) continue;   // skip occluders; don't drag avg down
        acc += cell_value(w, cx, cy, cz, daylight);
        s++;
    }
    if (s == 0) return cell_value(w, ox, oy, oz, daylight);
    return acc / (float)s;
}

void lp_sample_face_raw(world *w, int wx, int wy, int wz, int face,
                        uint8_t *out_block, uint8_t *out_sky) {
    const face_basis *f = &FACES[face];
    int ox = wx + f->nx, oy = wy + f->ny, oz = wz + f->nz;
    if (out_block) *out_block = lp_get_light(w, LP_BLOCK, ox, oy, oz);
    if (out_sky)   *out_sky   = lp_get_light(w, LP_SKY,   ox, oy, oz);
}
