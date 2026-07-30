#include "nav_los.h"
#include "nav_query.h"
#include <stdlib.h>
#include <math.h>
static int cell_near_y(const nav_grid *g, int x, int z, int want_y, int slack) {
    // probe outward from the expected height so the closest match wins, which
    // keeps the walk hugging the interpolated line on gentle slopes.
    for (int d = 0; d <= slack; d++) {
        int idx = nav_grid_find(g, nav_coord_make(x, want_y - d, z));
        if (idx >= 0) return idx;
        if (d) {
            idx = nav_grid_find(g, nav_coord_make(x, want_y + d, z));
            if (idx >= 0) return idx;
        }
    }
    return -1;
}

// integer 2d supercover-ish march from (x0,z0) to (x1,z1). at each block we
// call `visit`;
if it returns 0 the walk aborts and we report how far we got.
// returns 1 if the whole line was clear. expected floor height is lerped along
// the run so slopes track.
static int march(const nav_grid *g, int x0, int y0, int z0,
                 int x1, int y1, int z1, int slack,
                 int *last_ok_x, int *last_ok_z) {
    int dx = abs(x1 - x0), dz = abs(z1 - z0);
    int sx = x0 < x1 ? 1 : -1;
    int sz = z0 < z1 ? 1 : -1;
    int err = dx - dz;

    int x = x0, z = z0;
    int steps = dx + dz;          // upper bound on iterations
    int total = steps > 0 ? steps : 1;
    int taken = 0;

    *last_ok_x = x0;
    *last_ok_z = z0;

    for (;;) {
        // expected floor height: linear blend between the two endpoints by how
        // far along the manhattan span we are.
        float t = (float)taken / (float)total;
        int want_y = y0 + (int)lroundf((y1 - y0) * t);

        if (cell_near_y(g, x, z, want_y, slack) < 0) return 0;
        *last_ok_x = x;
        *last_ok_z = z;

        if (x == x1 && z == z1) return 1;

        int e2 = 2 * err;
        if (e2 > -dz) { err -= dz; x += sx; taken++; }
        if (e2 <  dx) { err += dx; z += sz; taken++; }
    }
}

int nav_los_cells(const nav_grid *g, int a, int b) {
    if (a < 0 || b < 0 || a >= g->count || b >= g->count) return 0;
if (a == b) return 1;
const nav_cell *ca = &g->cells[a];
const nav_cell *cb = &g->cells[b];
int ox, oz;
return march(g, ca->x, ca->y, ca->z, cb->x, cb->y, cb->z,
                 NAV_LOS_Y_SLACK, &ox, &oz);
if (b < 0 || b >= g->count) return a;
if (a == b) return a;
const nav_cell *ca = &g->cells[a];
const nav_cell *cb = &g->cells[b];
int ox = ca->x, oz = ca->z;
march(g, ca->x, ca->y, ca->z, cb->x, cb->y, cb->z,
          NAV_LOS_Y_SLACK, &ox, &oz);
int t = (abs(cb->x - ca->x) + abs(cb->z - ca->z));
int span = t > 0 ? t : 1;
int done = abs(ox - ca->x) + abs(oz - ca->z);
float frac = (float)done / (float)span;
int want_y = ca->y + (int)lroundf((cb->y - ca->y) * frac);
int idx = cell_near_y(g, ox, oz, want_y, NAV_LOS_Y_SLACK);
return idx >= 0 ? idx : a;
}
