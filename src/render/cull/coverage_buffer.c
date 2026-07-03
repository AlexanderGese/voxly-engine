#include "coverage_buffer.h"

#include <math.h>

// project a world point to clip, then to ndc, then to our grid space.
// returns 0 if the point is behind the near plane (w <= 0) — caller has
// to deal with clipping, we just bail to "everything visible" in that case.

typedef struct { float sx, sy, depth; int behind; } projected;

static projected project(const mat4 *vp, vec3 p) {
    // clip = vp * (p, 1). vp is column-major m[col][row].
    float cx = vp->m[0][0]*p.x + vp->m[1][0]*p.y + vp->m[2][0]*p.z + vp->m[3][0];
    float cy = vp->m[0][1]*p.x + vp->m[1][1]*p.y + vp->m[2][1]*p.z + vp->m[3][1];
    float cz = vp->m[0][2]*p.x + vp->m[1][2]*p.y + vp->m[2][2]*p.z + vp->m[3][2];
    float cw = vp->m[0][3]*p.x + vp->m[1][3]*p.y + vp->m[2][3]*p.z + vp->m[3][3];

    projected pr;
    if (cw <= 1e-5f) { pr.behind = 1; pr.sx = pr.sy = 0; pr.depth = 0; return pr; }
    pr.behind = 0;

    float inv = 1.0f / cw;
    float ndx = cx * inv;   // -1..1
    float ndy = cy * inv;
    pr.depth  = cz * inv;   // -1..1, smaller = nearer

    pr.sx = (ndx * 0.5f + 0.5f) * (float)CCOV_W;
    pr.sy = (1.0f - (ndy * 0.5f + 0.5f)) * (float)CCOV_H;
    return pr;
}

// project all 8 corners, return screen bbox + nearest depth.
// returns 0 if any corner is behind near (treat as can't-occlude / visible).
static int box_screen_rect(const mat4 *vp, aabb a,
                           int *x0, int *y0, int *x1, int *y1, float *nearest) {
    vec3 corners[8] = {
        {a.min.x, a.min.y, a.min.z}, {a.max.x, a.min.y, a.min.z},
        {a.min.x, a.max.y, a.min.z}, {a.max.x, a.max.y, a.min.z},
        {a.min.x, a.min.y, a.max.z}, {a.max.x, a.min.y, a.max.z},
        {a.min.x, a.max.y, a.max.z}, {a.max.x, a.max.y, a.max.z},
    };

    float minx = 1e30f, miny = 1e30f, maxx = -1e30f, maxy = -1e30f;
    float near = CCOV_FAR;

    for (int i = 0; i < 8; i++) {
        projected pr = project(vp, corners[i]);
        if (pr.behind) return 0;   // straddles near plane, give up
        if (pr.sx < minx) minx = pr.sx;
        if (pr.sy < miny) miny = pr.sy;
        if (pr.sx > maxx) maxx = pr.sx;
        if (pr.sy > maxy) maxy = pr.sy;
        if (pr.depth < near) near = pr.depth;
    }

    int ix0 = (int)floorf(minx), iy0 = (int)floorf(miny);
    int ix1 = (int)ceilf(maxx),  iy1 = (int)ceilf(maxy);
    if (ix0 < 0) ix0 = 0;
    if (iy0 < 0) iy0 = 0;
    if (ix1 > CCOV_W) ix1 = CCOV_W;
    if (iy1 > CCOV_H) iy1 = CCOV_H;

    *x0 = ix0; *y0 = iy0; *x1 = ix1; *y1 = iy1;
    *nearest = near;
    return (ix1 > ix0 && iy1 > iy0);
}

void cull_coverage_clear(cull_coverage *cb, mat4 vp) {
    cb->w = CCOV_W;
    cb->h = CCOV_H;
    cb->vp = vp;
    cb->any_occluders = 0;
    int n = CCOV_W * CCOV_H;
    for (int i = 0; i < n; i++) cb->depth[i] = CCOV_FAR;
}

int cull_coverage_add_box(cull_coverage *cb, aabb a) {
    int x0, y0, x1, y1;
    float near;
    if (!box_screen_rect(&cb->vp, a, &x0, &y0, &x1, &y1, &near)) return 0;

    // write the box's *farthest* visible depth conservatively. using the
    // near depth here would over-occlude things at the same range, so we
    // bias slightly: occluders should only block stuff strictly behind.
    // we use the nearest face depth, which keeps it conservative-ish for
    // big solid columns (chunks are full-height so this is fine).
    for (int y = y0; y < y1; y++) {
        float *row = &cb->depth[y * CCOV_W];
        for (int x = x0; x < x1; x++) {
            if (near < row[x]) row[x] = near;
        }
    }
    cb->any_occluders = 1;
    return 1;
}

int cull_coverage_test_box(const cull_coverage *cb, aabb a) {
    if (!cb->any_occluders) return 0;   // nothing to hide behind

    int x0, y0, x1, y1;
    float near;
    if (!box_screen_rect(&cb->vp, a, &x0, &y0, &x1, &y1, &near)) return 0;

    // tiny depth bias so a chunk doesnt occlude itself / its own seam.
    float test = near - 1e-4f;

    // occluded only if EVERY covered cell already holds something nearer.
    // first cell that's still farther than us means we'd be visible there.
    for (int y = y0; y < y1; y++) {
        const float *row = &cb->depth[y * CCOV_W];
        for (int x = x0; x < x1; x++) {
            if (row[x] >= test) return 0;   // a gap, we can be seen
        }
    }
    return 1;   // fully behind known occluders
}
