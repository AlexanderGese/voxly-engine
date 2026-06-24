#include "rivers_outlet.h"
#include "rivers_flow.h"
#include "rivers_rand.h"

// per-lake we want the single lowest rim cell, keyed by lake water level so two
// adjacent lakes at different heights don't share a saddle. we don't bother with
// proper lake ids: a saddle is uniquely a dry cell touching a lake, and the
// lowest such cell per water level is a good enough spillway for worldgen.

int rivers_outlet_carve(rivers_field *f, const rivers_params *p) {
    if (!p->carve_outlet) return 0;
    rivers_rng rng;
    rivers_rng_seed(&rng, rivers_seed_mix(p->seed, 0x52564F54u)); // 'RVOT'

    int carved = 0;

    for (int z = 1; z < RIVERS_DIM_Z - 1; z++) {
        for (int x = 1; x < RIVERS_DIM_X - 1; x++) {
            int idx = rivers_field_idx(x, z);
            if (f->wet[idx] != RIVERS_DRY) continue;

            // is this dry cell on a lake rim, and is it the spill height? a
            // spillway sits at most a block above the water it lets out.
            int adj_water = 0, best_level = 0;
            for (int d = 0; d < 8; d++) {
                int nx = x + rivers_dir_dx[d];
                int nz = z + rivers_dir_dz[d];
                int nidx = rivers_field_idx(nx, nz);
                if (f->wet[nidx] != RIVERS_LAKE) continue;
                adj_water = 1;
                if (f->water_y[nidx] > best_level) best_level = f->water_y[nidx];
            }
            if (!adj_water) continue;

            // only notch where the rim is right at the brim. higher rim cells
            // are real walls holding the lake in, leave them.
            if (f->surface[idx] > best_level + 1) continue;

            // does the terrain actually fall away on the far side? check the
            // cell opposite the lake; if it's lower, water has somewhere to run.
            int downhill = 0;
            for (int d = 0; d < 8; d++) {
                int nx = x + rivers_dir_dx[d];
                int nz = z + rivers_dir_dz[d];
                if (!rivers_field_in_bounds(nx, nz)) continue;
                int nidx = rivers_field_idx(nx, nz);
                if (f->wet[nidx] == RIVERS_LAKE) continue;
                if (f->surface[nidx] < best_level) { downhill = 1; break; }
            }
            if (!downhill) continue;

            // notch it: drop the rim to the water level and call it a river so
            // the carver cuts a channel and pours the overflow through.
            f->surface[idx] = best_level;
            f->water_y[idx] = best_level;
            f->wet[idx]     = RIVERS_RIVER;
            // a hair of accumulation so depth shaping treats it as a real,
            // if small, outflow.
            if (f->accum[idx] < (float)p->river_threshold)
                f->accum[idx] = (float)p->river_threshold +
                                rivers_rng_frange(&rng, 0.0f, 8.0f);
            carved++;
        }
    }

    if (carved) f->dirty = 1;
    return carved;
}
