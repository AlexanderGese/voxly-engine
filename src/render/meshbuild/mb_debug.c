#include "mb_debug.h"
#include "../../util/darray.h"
#include "../../util/log.h"

#include <math.h>

static int bad_float(float f) {
    return isnan(f) || isinf(f);
}

static int vert_nan(const mb_vertex *v) {
    return bad_float(v->x) || bad_float(v->y) || bad_float(v->z) ||
           bad_float(v->u) || bad_float(v->v) || bad_float(v->light);
}

// twice the triangle area via the cross product magnitude. we only care if its
// (near) zero, which means the three corners are colinear — a wasted tri.
static double tri_area2(const mb_vertex *a, const mb_vertex *b,
                        const mb_vertex *c) {
    double ux = b->x - a->x, uy = b->y - a->y, uz = b->z - a->z;
    double vx = c->x - a->x, vy = c->y - a->y, vz = c->z - a->z;
    double cx = uy * vz - uz * vy;
    double cy = uz * vx - ux * vz;
    double cz = ux * vy - uy * vx;
    return sqrt(cx * cx + cy * cy + cz * cz);
}

mb_check meshbuild_check(const mb_result *r) {
    mb_check chk = {0};
    chk.ok = 1;

    size_t nverts = darr_len(r->verts);
    size_t nidx   = darr_len(r->indices);

    // index count must be a whole number of triangles.
    if (nidx % 3 != 0) chk.ok = 0;

    for (size_t i = 0; i < nverts; i++) {
        const mb_vertex *v = &r->verts[i];
        if (vert_nan(v)) chk.nan_verts++;
        if (v->light < 0.0f || v->light > 1.0f) chk.bad_light++;
    }

    for (size_t i = 0; i + 2 < nidx; i += 3) {
        uint32_t ia = r->indices[i];
        uint32_t ib = r->indices[i + 1];
        uint32_t ic = r->indices[i + 2];

        if (ia >= nverts || ib >= nverts || ic >= nverts) {
            chk.oob_indices++;
            continue;   // cant area-check a tri that points out of bounds
        }
        if (tri_area2(&r->verts[ia], &r->verts[ib], &r->verts[ic]) < 1e-9)
            chk.degenerate_tris++;
    }

    if (chk.degenerate_tris || chk.oob_indices ||
        chk.nan_verts || chk.bad_light)
        chk.ok = 0;

    return chk;
}

int meshbuild_check_log(const mb_result *r) {
    mb_check c = meshbuild_check(r);
    if (c.ok) {
        LOGD("meshbuild ok: %d quads, %zu verts",
             r->quad_count, darr_len(r->verts));
    } else {
        LOGW("meshbuild issues: degenerate=%d oob=%d nan=%d badlight=%d",
             c.degenerate_tris, c.oob_indices, c.nan_verts, c.bad_light);
    }
    return c.ok;
}
