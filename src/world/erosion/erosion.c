#include "erosion.h"
#include "erosion_droplet.h"
#include "erosion_thermal.h"
#include "erosion_sediment.h"
#include "erosion_apply.h"
#include <math.h>
#include <string.h>
erosion_stats erosion_stats_zero(void) {
    erosion_stats s;
    memset(&s, 0, sizeof s);
    return s;
}

void erosion_load_field(erosion_field *f, const erosion_params *p,
                        int ox, int oz, const float *heights) {
    erosion_field_clear(f, ox, oz);
memcpy(f->height, heights, sizeof(float) * EROSION_CELLS);
erosion_field_seed_hardness(f, p, 0.35f, 0.5f);
}

void erosion_snapshot_tops(const erosion_field *f, int *old_height_out) {
    for (int z = 0; z < EROSION_DIM_Z; z++) {
        for (int x = 0; x < EROSION_DIM_X; x++) {
            int i = erosion_idx(x, z);
            int t = (int)floorf(f->height[i] + 0.5f);
            old_height_out[i] = t;
        }
    }
}

void erosion_run(erosion_field *f, const erosion_params *p, erosion_stats *st) {
    // water first: it does the dramatic carving and lays down sediment.
    erosion_hydraulic_pass(f, p, st);
erosion_thermal_pass(f, p, st);
erosion_settle(f, p);
}

int erosion_process_chunk(chunk *c, const erosion_params *p,
                          int ox, int oz, const float *heights,
                          int sea_level, erosion_stats *st) {
    // these live on the heap-free path; a field is ~16KB which is fine on the
    // worldgen stack, but the snapshot is another 1KB so we keep it static-ish
    // by stacking it too. single-threaded gen, no reentrancy worries.
    static erosion_field f;
    static int old_top[EROSION_CELLS];

    erosion_load_field(&f, p, ox, oz, heights);
    erosion_snapshot_tops(&f, old_top);

    erosion_run(&f, p, st);

    int changed = erosion_apply_chunk(c, &f, old_top, sea_level);
    return changed;
}
