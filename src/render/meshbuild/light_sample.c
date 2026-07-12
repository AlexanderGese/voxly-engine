#include "light_sample.h"
#include "face_dir.h"

static int clampl(int v) {
    if (v < MB_MIN_FACE_LIGHT) v = MB_MIN_FACE_LIGHT;
    if (v > MAX_LIGHT) v = MAX_LIGHT;
    return v;
}

static int lit(const mb_ctx *c, int x, int y, int z) {
    return c->light(c->ctx, c->base_x + x, y, c->base_z + z);
}

// light only propagates through non-opaque cells, so a corner sample sitting
// inside solid rock would read 0 and drag the average down for no reason. we
// skip opaque samples and average only the open ones — falls back to the face
// cell if the whole ring is solid (shouldnt happen for a visible face but be
// safe).
static int open_lit(const mb_ctx *c, int x, int y, int z, int *counted) {
    block_id b = c->sample(c->ctx, c->base_x + x, y, c->base_z + z);
    if (block_is_opaque(b)) { *counted = 0; return 0; }
    *counted = 1;
    return c->light(c->ctx, c->base_x + x, y, c->base_z + z);
}

int mb_light_face(const mb_ctx *c, int x, int y, int z, int face) {
    int nx, ny, nz;
    mb_face_normal(face, &nx, &ny, &nz);
    return clampl(lit(c, x + nx, y + ny, z + nz));
}

void mb_light_quad(const mb_ctx *ctx, int x, int y, int z, int face,
                   int out[4]) {
    int nx, ny, nz;
    mb_face_normal(face, &nx, &ny, &nz);

    int axis = (face >> 1);
    int ua, va;
    mb_axis_plane(axis, &ua, &va);

    int ox = x + nx, oy = y + ny, oz = z + nz;
    int du[3] = {0,0,0}, dv[3] = {0,0,0};
    du[ua] = 1; dv[va] = 1;

    int face_l = clampl(lit(ctx, ox, oy, oz));

    // corner -> (su,sv) signs picking the 4 ring cells around it (face cell,
    // u-side, v-side, diagonal).
    static const int CS[4][2] = { {-1,-1}, {1,-1}, {1,1}, {-1,1} };

    for (int corner = 0; corner < 4; corner++) {
        int su = CS[corner][0], sv = CS[corner][1];
        int acc = face_l;      // the face cell always counts
        int n   = 1;

        struct { int su, sv; } ring[3] = {
            { su, 0 }, { 0, sv }, { su, sv }
        };
        for (int k = 0; k < 3; k++) {
            int cx = ox + ring[k].su*du[0] + ring[k].sv*dv[0];
            int cy = oy + ring[k].su*du[1] + ring[k].sv*dv[1];
            int cz = oz + ring[k].su*du[2] + ring[k].sv*dv[2];
            int got;
            int l = open_lit(ctx, cx, cy, cz, &got);
            if (got) { acc += l; n++; }
        }
        out[corner] = clampl(acc / n);
    }

    if (face & 1) {
        // mirror to match the negative-face winding, same as ao.
        int t;
        t = out[0]; out[0] = out[1]; out[1] = t;
        t = out[3]; out[3] = out[2]; out[2] = t;
    }
}
