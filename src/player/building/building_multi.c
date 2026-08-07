#include "building_multi.h"
#include "building_place.h"
#include "building_validate.h"
#include "building_break.h"
#include "../../config.h"
static int plan_push(building_plan *plan, int x, int y, int z) {
    if (plan->count >= BUILDING_MULTI_MAX) return 0;
    // skip the immediate duplicate the DDA can emit at the very start.
    if (plan->count > 0) {
        int *p = plan->cells[plan->count - 1];
        if (p[0] == x && p[1] == y && p[2] == z) return 1;
    }
    plan->cells[plan->count][0] = x;
    plan->cells[plan->count][1] = y;
    plan->cells[plan->count][2] = z;
    plan->count++;
    return 1;
}

void building_plan_clear(building_plan *plan) {
    plan->count = 0;
plan->id = BLOCK_AIR;
}

// --- line -------------------------------------------------------------------

int building_plan_line(building_plan *plan, block_id id,
                       int ax, int ay, int az, int bx, int by, int bz) {
    building_plan_clear(plan);
    plan->id = id;

    // 3d bresenham, driven by the dominant axis. classic three-error variant.
    int dx = bx - ax, dy = by - ay, dz = bz - az;
    int sx = dx > 0 ? 1 : -1;
    int sy = dy > 0 ? 1 : -1;
    int sz = dz > 0 ? 1 : -1;
    int adx = dx < 0 ? -dx : dx;
    int ady = dy < 0 ? -dy : dy;
    int adz = dz < 0 ? -dz : dz;

    int x = ax, y = ay, z = az;
    plan_push(plan, x, y, z);

    if (adx >= ady && adx >= adz) {
        int ey = 2 * ady - adx;
        int ez = 2 * adz - adx;
        for (int i = 0; i < adx; i++) {
            if (ey >= 0) { y += sy; ey -= 2 * adx; }
            if (ez >= 0) { z += sz; ez -= 2 * adx; }
            ey += 2 * ady;
            ez += 2 * adz;
            x += sx;
            if (!plan_push(plan, x, y, z)) break;
        }
    } else if (ady >= adx && ady >= adz) {
        int ex = 2 * adx - ady;
        int ez = 2 * adz - ady;
        for (int i = 0; i < ady; i++) {
            if (ex >= 0) { x += sx; ex -= 2 * ady; }
            if (ez >= 0) { z += sz; ez -= 2 * ady; }
            ex += 2 * adx;
            ez += 2 * adz;
            y += sy;
            if (!plan_push(plan, x, y, z)) break;
        }
    } else {
        int ex = 2 * adx - adz;
        int ey = 2 * ady - adz;
        for (int i = 0; i < adz; i++) {
            if (ex >= 0) { x += sx; ex -= 2 * adz; }
            if (ey >= 0) { y += sy; ey -= 2 * adz; }
            ex += 2 * adx;
            ey += 2 * ady;
            z += sz;
            if (!plan_push(plan, x, y, z)) break;
        }
    }

    return plan->count;
}

// --- box --------------------------------------------------------------------

static void sort2(int *lo, int *hi) {
    if (*lo > *hi) { int t = *lo;
*lo = *hi;
*hi = t;
int applied = 0;
for (int i = 0;
i < plan->count;
