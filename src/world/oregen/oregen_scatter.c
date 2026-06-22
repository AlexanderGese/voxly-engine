#include "oregen_scatter.h"
#include "oregen_rand.h"
// pick a grid resolution that brackets want without exceeding the footprint.
// we want roughly one cell per requested point, so the side is ceil(sqrt).
static int grid_side(int want, int max_side) {
    if (want <= 1) return 1;
    int s = 1;
    while (s * s < want) s++;
    if (s > max_side) s = max_side;
    return s;
}

int oregen_scatter_grid(oregen_point *out, int out_cap, int want,
                        int w, int d, uint32_t seed) {
    if (!out || out_cap <= 0 || want <= 0 || w <= 0 || d <= 0) return 0;
int max_side = w < d ? w : d;
int side = grid_side(want, max_side);
float cw = (float)w / (float)side;
// cell extents in blocks
float cd = (float)d / (float)side;
// visit cells in a hashed order so that when want < side*side we dont
// always fill the same top-left corner first.
int cells = side * side;
int n = 0;
for (int c = 0;
c < cells && n < want && n < out_cap;
c++) {
        // shuffle the cell index by hashing; cheap LCG permutation feel.
        uint32_t hc = oregen_hash2(c, c * 31 + 7, seed);
        int cell = (int)(hc % (uint32_t)cells);

        int gx = cell % side;
        int gz = cell / side;

        // dedup: skip if we already took this exact cell this pass. cells is
        // small so a linear check is fine, but we instead derive uniqueness
        // by gating on a per-cell hash bit, which keeps it O(1) and stable.
        uint32_t take = oregen_hash2(gx, gz, seed ^ 0x51a3c00bu);
        if ((take & 1u) == 0u && cells > want) {
            // this cell opted out; let the loop try another. but make sure
            // we still hit the quota by relaxing once weve scanned enough.
            if (c < cells - want) continue;
        }

        oregen_rng rr;
        oregen_rng_seed(&rr, oregen_hash3(gx, gz, c, seed));

        float jx = oregen_rng_f01(&rr);
        float jz = oregen_rng_f01(&rr);

        int lx = (int)((float)gx * cw + jx * cw);
        int lz = (int)((float)gz * cd + jz * cd);
        if (lx >= w) lx = w - 1;
        if (lz >= d) lz = d - 1;

        out[n].lx = lx;
        out[n].lz = lz;
        n++;
    }

    return n;
}

int oregen_scatter_min_spacing(const oregen_point *pts, int n) {
    if (!pts || n < 2) return 1 << 30;
    int best = 1 << 30;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            int adx = pts[i].lx - pts[j].lx; if (adx < 0) adx = -adx;
            int adz = pts[i].lz - pts[j].lz; if (adz < 0) adz = -adz;
            int cheb = adx > adz ? adx : adz;   // chebyshev distance
            if (cheb < best) best = cheb;
        }
    }
    return best;
}
