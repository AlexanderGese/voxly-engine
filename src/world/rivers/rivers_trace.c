#include "rivers_trace.h"
#include "rivers_flow.h"
#include "../../util/darray.h"

#include <math.h>

// a cell is "wet enough" to be a river if its accumulation cleared threshold.
static int over_threshold(const rivers_field *f, int idx,
                          const rivers_params *p) {
    return f->accum[idx] >= (float)p->river_threshold;
}

int rivers_trace_mark(rivers_field *f, const rivers_params *p) {
    int rivers = 0;

    for (int z = 0; z < RIVERS_DIM_Z; z++) {
        for (int x = 0; x < RIVERS_DIM_X; x++) {
            int idx = rivers_field_idx(x, z);
            if (!over_threshold(f, idx, p)) continue;
            if (f->wet[idx] == RIVERS_LAKE) continue;   // lake already owns it

            // springs that would sit underwater are pointless; suppress them.
            if (f->surface[idx] < p->min_source_y) continue;

            // is any upstream neighbour also a river? a cell is a SOURCE only if
            // it's the first over-threshold cell in its flow line. we check the
            // neighbours that point *at* us.
            int has_upstream = 0;
            for (int d = 0; d < 8; d++) {
                int nx = x + rivers_dir_dx[d];
                int nz = z + rivers_dir_dz[d];
                if (!rivers_field_in_bounds(nx, nz)) continue;
                int nidx = rivers_field_idx(nx, nz);
                rivers_dir nd = (rivers_dir)f->dir[nidx];
                if (nd == RIVERS_DIR_NONE) continue;
                // does that neighbour drain into us, and is it itself a river?
                if (nx + rivers_dir_dx[nd] == x &&
                    nz + rivers_dir_dz[nd] == z &&
                    over_threshold(f, nidx, p)) {
                    has_upstream = 1;
                    break;
                }
            }

            f->wet[idx] = has_upstream ? RIVERS_RIVER : RIVERS_SOURCE;
            rivers++;
        }
    }
    if (rivers) f->dirty = 1;
    return rivers;
}

int rivers_trace_path(rivers_field *f, const rivers_params *p,
                      int src_x, int src_z, rivers_vertex **path) {
    int x = src_x, z = src_z;
    int appended = 0;

    for (int step = 0; step < p->max_trace_steps; step++) {
        int idx = rivers_field_idx(x, z);

        rivers_vertex v;
        v.x         = x;
        v.z         = z;
        v.surface_y = f->surface[idx];
        v.accum     = f->accum[idx];
        v.water_y   = f->water_y[idx] ? f->water_y[idx] : f->surface[idx];
        darr_push(*path, v);
        appended++;

        // a lake is the end of the line; the river pours into it.
        if (f->wet[idx] == RIVERS_LAKE) break;

        rivers_dir d = (rivers_dir)f->dir[idx];
        if (d == RIVERS_DIR_NONE) break;            // sink, nowhere to go

        int nx = x + rivers_dir_dx[d];
        int nz = z + rivers_dir_dz[d];
        if (!rivers_field_in_bounds(nx, nz)) break; // ran off the region

        // guard against a flat 2-cycle: if the next cell points right back at
        // us, bail rather than ping-pong forever.
        rivers_dir nd = (rivers_dir)f->dir[rivers_field_idx(nx, nz)];
        if (nd != RIVERS_DIR_NONE &&
            nx + rivers_dir_dx[nd] == x &&
            nz + rivers_dir_dz[nd] == z)
            break;

        x = nx;
        z = nz;
    }
    return appended;
}

int rivers_trace_resolve_levels(rivers_field *f, const rivers_params *p) {
    int changed = 0;

    // seed: every wet cell starts at its own surface, lakes keep their fill
    // level, and anything below sea level is clamped up to the sea.
    for (int i = 0; i < RIVERS_CELLS; i++) {
        if (f->wet[i] == RIVERS_DRY) continue;
        if (f->wet[i] == RIVERS_LAKE) continue;     // fill already set water_y
        int wy = f->surface[i];
        if (wy < p->sea_level) wy = p->sea_level;
        f->water_y[i] = wy;
    }

    // relax downstream: a cell's water surface can't be higher than the cell it
    // flows into (no uphill water). iterate until stable; the field is small so
    // a handful of passes converge. cap the passes so a pathological loop can't
    // wedge us.
    for (int pass = 0; pass < 16; pass++) {
        int dirty = 0;
        for (int z = 0; z < RIVERS_DIM_Z; z++) {
            for (int x = 0; x < RIVERS_DIM_X; x++) {
                int idx = rivers_field_idx(x, z);
                if (f->wet[idx] == RIVERS_DRY) continue;

                rivers_dir d = (rivers_dir)f->dir[idx];
                if (d == RIVERS_DIR_NONE) continue;
                int nx = x + rivers_dir_dx[d];
                int nz = z + rivers_dir_dz[d];
                if (!rivers_field_in_bounds(nx, nz)) continue;
                int nidx = rivers_field_idx(nx, nz);
                if (f->wet[nidx] == RIVERS_DRY) continue;

                // downstream must sit at or below us; if we're below it, lift us
                // to match so the surface is monotone.
                if (f->water_y[idx] < f->water_y[nidx]) {
                    f->water_y[idx] = f->water_y[nidx];
                    dirty = 1;
                }
            }
        }
        if (!dirty) break;
        changed = 1;
    }
    return changed;
}
