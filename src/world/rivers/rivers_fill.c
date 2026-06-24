#include "rivers_fill.h"
#include "rivers_flow.h"   // for the dir step tables
#include "rivers_heap.h"

#include <stdlib.h>
#include <string.h>

// scratch flags so we don't push a cell onto the heap twice, then reused as a
// basin-claim plane in the lake sweep. lives here, sized to the field.
static uint8_t s_closed[RIVERS_CELLS];

// lake cap travels via a file-static so the basin flood reads cleanly. set once
// at the top of rivers_fill_run, never concurrently (one region per thread).
static int s_fill_max_y;

void rivers_fill_depressions(rivers_field *f, const rivers_params *p) {
    (void)p;
    rivers_heap heap;
    rivers_heap_init(&heap);
    memset(s_closed, 0, sizeof(s_closed));

    // seed the frontier with every boundary cell at its own height. the edge is
    // where water leaves the region, so it can never be a pit.
    for (int z = 0; z < RIVERS_DIM_Z; z++) {
        for (int x = 0; x < RIVERS_DIM_X; x++) {
            if (!rivers_field_on_edge(x, z)) continue;
            int idx = rivers_field_idx(x, z);
            f->filled[idx] = f->surface[idx];
            s_closed[idx]  = 1;
            rivers_heap_push(&heap, idx, f->filled[idx]);
        }
    }

    // grow inward from the lowest frontier cell. the spill height is the key we
    // just popped; any lower neighbour gets lifted to it.
    while (!rivers_heap_empty(&heap)) {
        int spill;
        int idx = rivers_heap_pop(&heap, &spill);
        int x = idx % RIVERS_DIM_X, z = idx / RIVERS_DIM_X;

        for (int d = 0; d < 8; d++) {
            int nx = x + rivers_dir_dx[d];
            int nz = z + rivers_dir_dz[d];
            if (!rivers_field_in_bounds(nx, nz)) continue;
            int nidx = rivers_field_idx(nx, nz);
            if (s_closed[nidx]) continue;

            int nh = f->surface[nidx];
            // if the neighbour sits below the spill height it's inside a basin;
            // raise it to the spill so water would brim over to here.
            int filled = nh < spill ? spill : nh;
            f->filled[nidx] = filled;
            s_closed[nidx]  = 1;
            rivers_heap_push(&heap, nidx, filled);
        }
    }
}

// flood one connected pond from a seed, collecting its members. returns count
// and the pond's water surface height; does not write the wet plane yet so the
// caller can apply min_lake_cells before committing.
static int flood_basin(rivers_field *f, int seed, int *members,
                       int *out_water_y) {
    int *stack = malloc(sizeof(int) * RIVERS_CELLS);
    if (!stack) return 0;
    int top = 0, count = 0;

    int water_y = f->filled[seed];
    stack[top++] = seed;
    s_closed[seed] = 2;   // 2 == claimed by this basin sweep

    while (top > 0) {
        int idx = stack[--top];
        members[count++] = idx;
        int x = idx % RIVERS_DIM_X, z = idx / RIVERS_DIM_X;

        for (int d = 0; d < 8; d++) {
            int nx = x + rivers_dir_dx[d];
            int nz = z + rivers_dir_dz[d];
            if (!rivers_field_in_bounds(nx, nz)) continue;
            int nidx = rivers_field_idx(nx, nz);
            if (s_closed[nidx] == 2) continue;
            if (f->filled[nidx] <= f->surface[nidx]) continue;  // not ponded
            if (f->filled[nidx] != water_y) continue;           // diff pond level
            s_closed[nidx] = 2;
            stack[top++] = nidx;
        }
    }

    free(stack);
    if (out_water_y) *out_water_y = water_y;
    return count;
}

int rivers_fill_run(rivers_field *f, const rivers_params *p) {
    s_fill_max_y = p->fill_max_y;

    rivers_fill_depressions(f, p);

    // reuse s_closed as the basin-claim plane. priority-flood left 1s in it;
    // wipe so the sweep starts clean and uses value 2 exclusively.
    memset(s_closed, 0, sizeof(s_closed));

    int *members = malloc(sizeof(int) * RIVERS_CELLS);
    if (!members) return 0;

    int total = 0;
    for (int z = 0; z < RIVERS_DIM_Z; z++) {
        for (int x = 0; x < RIVERS_DIM_X; x++) {
            int idx = rivers_field_idx(x, z);
            if (s_closed[idx] == 2) continue;
            if (f->filled[idx] <= f->surface[idx]) continue;   // not ponded

            int wy;
            int n = flood_basin(f, idx, members, &wy);

            // small puddles and ponds above the cap get dropped. the carve pass
            // will then see dry pits there and leave them be.
            if (n < p->min_lake_cells || wy > s_fill_max_y) continue;

            for (int i = 0; i < n; i++) {
                int m = members[i];
                if (wy > f->surface[m]) {
                    f->wet[m]     = RIVERS_LAKE;
                    f->water_y[m] = wy;
                }
            }
            total += n;
            f->dirty = 1;
        }
    }

    free(members);
    return total;
}
