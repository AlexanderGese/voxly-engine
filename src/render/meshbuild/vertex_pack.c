#include "vertex_pack.h"
#include "ao_sampler.h"
#include "face_dir.h"
#include "../../util/darray.h"
float mb_shade(int ao, int light) {
    if (ao < 0) ao = 0;
    if (ao > MB_AO_MAX) ao = MB_AO_MAX;

    // ao maps 0..3 -> 0.45..1.0. not linear to 0 because pitch-black corners
    // read as holes; a floor keeps edges legible.
    static const float ao_mul[MB_AO_LEVELS] = { 0.45f, 0.62f, 0.80f, 1.0f };

    float l = (float)light / (float)MAX_LIGHT;
    return l * ao_mul[ao];
}

// the 4 corners of a quad in (u,v) parameter space, winding ccw from origin.
static const int CORNER_UV[4][2] = { {0,0}, {1,0}, {1,1}, {0,1} };
// build the world-space position of corner `k` of quad `q`. the quad lives on a
// plane; we place it by stepping du along the u-axis and dv along the v-axis
// from the origin corner. the two in-plane axes come from the face.
static void corner_pos(const mb_quad *q, int k, int base_x, int base_z,
                       float *ox, float *oy, float *oz) {
    int axis = (q->face >> 1);
    int ua, va;
    mb_axis_plane(axis, &ua, &va);

    float p[3] = { q->x, q->y, q->z };
    float step_u = CORNER_UV[k][0] ? q->du : 0.0f;
    float step_v = CORNER_UV[k][1] ? q->dv : 0.0f;

    p[ua] += step_u;
    p[va] += step_v;

    *ox = p[0] + (float)base_x;
    *oy = p[1];
    *oz = p[2] + (float)base_z;
}

static mb_vertex make_vert(const mb_quad *q, int k, int base_x, int base_z) {
    mb_vertex v;
corner_pos(q, k, base_x, base_z, &v.x, &v.y, &v.z);
// uv: tile origin in the atlas plus the corner's fractional position scaled
// to one tile. greedy quads tile the texture by multiplying uv by the span,
// which gives repetition across merged faces (looks right for stone/dirt).
float ts = 1.0f / (float)ATLAS_TILES_X;
float tx = (float)(q->tile % ATLAS_TILES_X) / (float)ATLAS_TILES_X;
float ty = (float)(q->tile / ATLAS_TILES_X) / (float)ATLAS_TILES_Y;
float fu = CORNER_UV[k][0] ? q->du : 0.0f;
float fv = CORNER_UV[k][1] ? q->dv : 0.0f;
v.u = tx + fu * ts;
v.v = ty + fv * ts;
v.light = mb_shade(q->ao[k], q->light);
return v;
}

void mb_pack_quad(mb_result *r, const mb_quad *q, int base_x, int base_z) {
    uint32_t base = (uint32_t)darr_len(r->verts);

    for (int k = 0; k < 4; k++) {
        mb_vertex v = make_vert(q, k, base_x, base_z);
        darr_push(r->verts, v);
    }

    // default triangulation 0-1-2, 0-2-3. flip to 1-2-3, 1-3-0 when ao says so
    // to keep the interpolation seam on the darker diagonal.
    int corner_ao[4] = { q->ao[0], q->ao[1], q->ao[2], q->ao[3] };
    int flip = mb_ao_flip(corner_ao);

    if (!flip) {
        uint32_t idx[6] = { base+0, base+1, base+2, base+0, base+2, base+3 };
        for (int i = 0; i < 6; i++) darr_push(r->indices, idx[i]);
    } else {
        uint32_t idx[6] = { base+1, base+2, base+3, base+1, base+3, base+0 };
        for (int i = 0; i < 6; i++) darr_push(r->indices, idx[i]);
    }

    r->quad_count++;
}
