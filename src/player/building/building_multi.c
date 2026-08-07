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
    if (*lo > *hi) { int t = *lo; *lo = *hi; *hi = t; }
}

int building_plan_box(building_plan *plan, block_id id,
                      int ax, int ay, int az, int bx, int by, int bz,
                      int hollow) {
    building_plan_clear(plan);
    plan->id = id;

    int x0 = ax, x1 = bx; sort2(&x0, &x1);
    int y0 = ay, y1 = by; sort2(&y0, &y1);
    int z0 = az, z1 = bz; sort2(&z0, &z1);

    // bail before we walk a box that can't fit. cheap volume check.
    long vol = (long)(x1 - x0 + 1) * (y1 - y0 + 1) * (z1 - z0 + 1);
    if (vol > BUILDING_MULTI_MAX) return -1;

    for (int y = y0; y <= y1; y++) {
        for (int z = z0; z <= z1; z++) {
            for (int x = x0; x <= x1; x++) {
                if (hollow) {
                    // shell only: a cell is on the shell if it touches a min or
                    // max plane on any axis.
                    int on_shell = (x == x0 || x == x1 ||
                                    y == y0 || y == y1 ||
                                    z == z0 || z == z1);
                    if (!on_shell) continue;
                }
                if (!plan_push(plan, x, y, z)) return plan->count;
            }
        }
    }
    return plan->count;
}

// --- commit -----------------------------------------------------------------

int building_plan_commit(world *w, building_history *hist,
                         const building_plan *plan, vec3 feet) {
    if (plan->id == BLOCK_AIR) return 0;
    int applied = 0;

    for (int i = 0; i < plan->count; i++) {
        int x = plan->cells[i][0];
        int y = plan->cells[i][1];
        int z = plan->cells[i][2];

        // face here is meaningless for a bulk fill, so feed PY (support checks
        // that care about a clicked wall will just reject — fine for fills).
        int verdict = building_validate_place(w, plan->id, x, y, z, BFACE_PY, feet);
        if (verdict != BPLACE_OK) continue;

        block_id before = world_get_block(w, x, y, z);
        world_set_block(w, x, y, z, plan->id);
        building_mark_dirty(w, x, y, z);

        building_edit e = { x, y, z, before, plan->id };
        if (hist) building_history_record(hist, &e);
        applied++;
    }
    return applied;
}

int building_plan_erase(world *w, building_history *hist,
                        const building_plan *plan) {
    int cleared = 0;

    for (int i = 0; i < plan->count; i++) {
        int x = plan->cells[i][0];
        int y = plan->cells[i][1];
        int z = plan->cells[i][2];

        if (y < 0 || y >= CHUNK_SIZE_Y) continue;

        block_id id = world_get_block(w, x, y, z);
        if (!building_is_breakable(id)) continue;

        world_set_block(w, x, y, z, BLOCK_AIR);
        building_mark_dirty(w, x, y, z);

        building_edit e = { x, y, z, id, BLOCK_AIR };
        if (hist) building_history_record(hist, &e);
        cleared++;
    }
    return cleared;
}
