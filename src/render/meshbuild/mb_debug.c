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
if (nidx % 3 != 0) chk.ok = 0;
for (size_t i = 0;
i < nverts;
i + 2 < nidx;
return chk;
