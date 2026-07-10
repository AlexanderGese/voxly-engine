#include "lt_debug.h"

#include "../../util/darray.h"
#include "../../world/block.h"
#include <math.h>
#include <stddef.h>

// the same six offsets the surface pass uses. duplicated here so the validator
// stays independent of lt_surface internals — if that file's table ever drifts,
// the expected-face count here will disagree and the test will yell.
static const int FACE_OFF[LT_FACE_COUNT][3] = {
    {  1,  0,  0 }, { -1,  0,  0 },
    {  0,  1,  0 }, {  0, -1,  0 },
    {  0,  0,  1 }, {  0,  0, -1 },
};

static int is_nan_f(float v) {
    // self-compare trick; works without enabling fast-math weirdness.
    return v != v;
}

int lt_debug_validate_mesh(const lt_mesh *m, const char **reason) {
    size_t vcount = darr_len(m->verts);
    size_t icount = darr_len(m->indices);

    if (icount % 3 != 0) {
        if (reason) *reason = "index count not a multiple of 3";
        return 0;
    }

    for (size_t i = 0; i < icount; i++) {
        if (m->indices[i] >= vcount) {
            if (reason) *reason = "index out of vertex range";
            return 0;
        }
    }

    for (size_t i = 0; i < vcount; i++) {
        const lt_vertex *v = &m->verts[i];
        if (is_nan_f(v->x) || is_nan_f(v->y) || is_nan_f(v->z) ||
            is_nan_f(v->u) || is_nan_f(v->v) || is_nan_f(v->light)) {
            if (reason) *reason = "NaN in vertex";
            return 0;
        }
        if (v->light < 0.0f || v->light > 1.0f) {
            if (reason) *reason = "light outside 0..1";
            return 0;
        }
    }

    // each quad is 4 verts / 6 indices; counts should stay in that ratio.
    if (vcount != 0 && (icount / 6) * 4 != vcount) {
        if (reason) *reason = "vertex/index ratio off (not pure quads)";
        return 0;
    }

    if (reason) *reason = NULL;
    return 1;
}

int lt_debug_validate_grid(const lt_grid *g) {
    if (g->step != LT_STEP(g->level)) return 0;
    if (g->nx <= 0 || g->ny <= 0 || g->nz <= 0) return 0;
    if (!g->cells) return 0;

    int vol = g->nx * g->ny * g->nz;
    for (int i = 0; i < vol; i++) {
        if (g->cells[i] >= BLOCK_COUNT) return 0;
    }
    return 1;
}

vec3 lt_debug_level_color(int level) {
    if (level < 0) level = 0;
    if (level >= LT_LEVEL_COUNT) level = LT_LEVEL_COUNT - 1;

    // lerp green -> red across the levels. t=0 cheap/near, t=1 coarse/far.
    float t = (LT_LEVEL_COUNT > 1)
            ? (float)level / (float)(LT_LEVEL_COUNT - 1)
            : 0.0f;

    vec3 c;
    c.x = t;            // red rises with coarseness
    c.y = 1.0f - t;     // green fades out
    c.z = 0.15f;        // a touch of blue so the darkest band isn't pure black
    return c;
}

int lt_debug_expected_faces(const lt_grid *g) {
    int faces = 0;
    for (int y = 0; y < g->ny; y++) {
        for (int z = 0; z < g->nz; z++) {
            for (int x = 0; x < g->nx; x++) {
                if (!lt_grid_cell_solid(g, x, y, z)) continue;
                for (int f = 0; f < LT_FACE_COUNT; f++) {
                    int nx = x + FACE_OFF[f][0];
                    int ny = y + FACE_OFF[f][1];
                    int nz = z + FACE_OFF[f][2];
                    if (!lt_grid_cell_solid(g, nx, ny, nz)) faces++;
                }
            }
        }
    }
    return faces;
}
