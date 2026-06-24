#include "rivers_carve.h"
#include "rivers_flow.h"
#include "../../util/darray.h"
#include "../../config.h"

#include <math.h>

int rivers_carve_depth(const rivers_params *p, float accum) {
    if (accum < (float)p->river_threshold) return 1;
    // depth grows with log2 of accumulation so a trunk river is deeper than a
    // tributary without the deltas exploding on huge catchments.
    float over = accum / (float)p->river_threshold;
    int d = 1 + (int)(p->depth_per_log * (log2f(over) + 1.0f));
    if (d < 1) d = 1;
    if (d > p->max_depth) d = p->max_depth;
    return d;
}

int rivers_carve_mark_banks(rivers_field *f, const rivers_params *p) {
    int marked = 0;
    int w = p->bank_width;
    if (w < 1) return 0;

    // a dry cell within bank_width of a river/lake/source cell becomes a bank.
    // we scan a small box per wet cell rather than the whole grid per dry cell;
    // bank_width is tiny so this stays cheap.
    for (int z = 0; z < RIVERS_DIM_Z; z++) {
        for (int x = 0; x < RIVERS_DIM_X; x++) {
            int idx = rivers_field_idx(x, z);
            uint8_t s = f->wet[idx];
            if (s != RIVERS_RIVER && s != RIVERS_LAKE && s != RIVERS_SOURCE)
                continue;

            for (int dz = -w; dz <= w; dz++) {
                for (int dx = -w; dx <= w; dx++) {
                    if (!dx && !dz) continue;
                    int nx = x + dx, nz = z + dz;
                    if (!rivers_field_in_bounds(nx, nz)) continue;
                    int nidx = rivers_field_idx(nx, nz);
                    if (f->wet[nidx] != RIVERS_DRY) continue;
                    // dont sand a cliff that towers over the water; banks are
                    // for shores roughly at water level.
                    if (f->surface[nidx] > f->water_y[idx] + 2) continue;
                    f->wet[nidx] = RIVERS_BANK;
                    marked++;
                }
            }
        }
    }
    return marked;
}

// push one edit, world-space.
static void emit(rivers_cell **out, int wx, int wy, int wz, block_id id) {
    rivers_cell c = { wx, wy, wz, id };
    darr_push(*out, c);
}

int rivers_carve_emit(rivers_field *f, const rivers_params *p,
                      rivers_cell **out) {
    int emitted = 0;

    for (int z = 0; z < RIVERS_DIM_Z; z++) {
        for (int x = 0; x < RIVERS_DIM_X; x++) {
            int idx = rivers_field_idx(x, z);
            uint8_t s = f->wet[idx];
            if (s == RIVERS_DRY) continue;

            int wx, wz;
            rivers_field_to_world(f, x, z, &wx, &wz);

            int surf    = f->surface[idx];
            int water_y = f->water_y[idx] ? f->water_y[idx] : surf;

            if (s == RIVERS_BANK) {
                // just retop the shore with sand. one edit, at the surface.
                emit(out, wx, surf, wz, BLOCK_SAND);
                emitted++;
                continue;
            }

            // river / lake / source all carve a bed then pour water on top.
            int depth;
            if (s == RIVERS_SOURCE) depth = 1;
            else if (s == RIVERS_RIVER) depth = rivers_carve_depth(p, f->accum[idx]);
            else depth = 1;   // lake floor is the basin floor, no extra cut

            int bed_y = (s == RIVERS_LAKE) ? surf : (water_y - depth);
            if (bed_y < 0) bed_y = 0;
            if (bed_y > water_y) bed_y = water_y;   // never invert

            // 1. carve the channel: clear everything from the old surface down
            // to the bed (air it out so the water column isn't blocked).
            for (int y = surf; y > bed_y; y--) {
                emit(out, wx, y, wz, BLOCK_AIR);
                emitted++;
            }

            // 2. lay a sand bed one block under the lowest water.
            emit(out, wx, bed_y, wz, BLOCK_SAND);
            emitted++;

            // 3. fill water from bed+1 up to and including water_y.
            for (int y = bed_y + 1; y <= water_y; y++) {
                emit(out, wx, y, wz, BLOCK_WATER);
                emitted++;
            }
        }
    }

    if (emitted) f->dirty = 1;
    return emitted;
}
