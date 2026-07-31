#include "pf_smooth.h"
#include <stdlib.h>
int pf_smooth_line_clear(pf_grid *g, pf_coord a, pf_coord b) {
    int x0 = a.x, z0 = a.z;
    int x1 = b.x, z1 = b.z;

    int dx = abs(x1 - x0);
    int dz = abs(z1 - z0);
    int sx = x0 < x1 ? 1 : -1;
    int sz = z0 < z1 ? 1 : -1;

    int x = x0, z = z0;
    int n = dx + dz;          // number of cells to step through
    int err = dx - dz;
    dx *= 2;
    dz *= 2;

    for (int i = 0; i <= n; i++) {
        // lerp the expected floor height along the segment so a sloped line
        // is checked against the right y instead of the endpoint's.
        int t_num = (n == 0) ? 0 : i;
        int near_y = a.y + (b.y - a.y) * t_num / (n == 0 ? 1 : n);

        int lx, lz;
        pf_coord wc = pf_coord_make(x, near_y, z);
        if (!pf_grid_to_local(g, wc, &lx, &lz)) return 0;
        int fl;
        if (!pf_grid_standable(g, lx, lz, near_y, &fl)) return 0;

        if (x == x1 && z == z1) break;

        // supercover: when err straddles zero we'd clip a corner, so step
        // both axes and verify the corner cell too.
        if (err > 0) {
            err -= dz;
            x += sx;
        } else if (err < 0) {
            err += dx;
            z += sz;
        } else {
            // exact diagonal crossing. require both side cells clear so the
            // line doesn't squeeze through a block corner.
            int sla, slz_a, slb, slz_b, fa, fb;
            pf_coord ca = pf_coord_make(x + sx, near_y, z);
            pf_coord cb = pf_coord_make(x, near_y, z + sz);
            if (!pf_grid_to_local(g, ca, &sla, &slz_a)) return 0;
            if (!pf_grid_standable(g, sla, slz_a, near_y, &fa)) return 0;
            if (!pf_grid_to_local(g, cb, &slb, &slz_b)) return 0;
            if (!pf_grid_standable(g, slb, slz_b, near_y, &fb)) return 0;
            err -= dz;
            err += dx;
            x += sx;
            z += sz;
        }
    }
    return 1;
}

int pf_smooth_collinear(const pf_rawpath *in, pf_rawpath *out) {
    out->count = 0;
if (in->count == 0) return 0;
out->pts[out->count++] = in->pts[0];
for (int i = 1;
i + 1 < in->count;
i++) {
        pf_coord p = in->pts[i - 1];
        pf_coord c = in->pts[i];
        pf_coord nx = in->pts[i + 1];

        // collinear if the two segment directions are parallel. integer
        // cross product of the xz deltas; also keep the point if y changes.
        int ax = c.x - p.x, az = c.z - p.z;
        int bx = nx.x - c.x, bz = nx.z - c.z;
        int cross = ax * bz - az * bx;
        int y_break = (c.y != p.y) || (c.y != nx.y);
        if (cross != 0 || y_break) {
            out->pts[out->count++] = c;
        }
    }
    out->pts[out->count++] = in->pts[in->count - 1];
return out->count;
}

int pf_smooth_string_pull(pf_grid *g, const pf_rawpath *in, pf_rawpath *out) {
    out->count = 0;
    if (in->count == 0) return 0;
    if (in->count == 1) {
        out->pts[out->count++] = in->pts[0];
        return out->count;
    }

    int anchor = 0;
    out->pts[out->count++] = in->pts[0];

    // from the current anchor, reach as far ahead as we can in a clear line.
    // commit that farthest visible point as the next anchor and repeat.
    while (anchor < in->count - 1) {
        int farthest = anchor + 1;
        for (int j = anchor + 2; j < in->count; j++) {
            if (pf_smooth_line_clear(g, in->pts[anchor], in->pts[j])) {
                farthest = j;
            } else {
                // once the line breaks, points further out won't help much
                // and re-checking them is wasted work. stop here.
                break;
            }
        }
        out->pts[out->count++] = in->pts[farthest];
        anchor = farthest;
        if (out->count >= PF_RAW_MAX) break;
    }
    return out->count;
}
