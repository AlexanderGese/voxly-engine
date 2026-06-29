#include "structgen_village.h"
#include "structgen_rand.h"

// the village is laid out on a coarse plot grid centered on the anchor. the
// well claims the center plot; the rest get houses or farms with some left
// empty as "courtyard". plots are bigger than the buildings so there's road
// space between them, which we dont voxelize (the terrain grass is the road).

#define PLOT       9     // plot pitch in blocks
#define GRID_RAD   2     // plots out from center each way -> 5x5 grid

static void plot_origin(const structgen_site *s, int gx, int gz, int *ox, int *oz) {
    // center the grid on the anchor. gx,gz in [-GRID_RAD, GRID_RAD].
    *ox = s->anchor_x + gx * PLOT - PLOT / 2;
    *oz = s->anchor_z + gz * PLOT - PLOT / 2;
}

void structgen_village_layout(structgen_plan *plan, const structgen_site *site) {
    structgen_rng rng;
    structgen_rng_seed(&rng, site->seed);

    int gy = site->ground_y;
    structgen_plan_init(plan, site->anchor_x, gy, site->anchor_z);

    // 1. the well in the center plot.
    {
        int ox, oz;
        plot_origin(site, 0, 0, &ox, &oz);
        structgen_box fp = structgen_box_at(ox + 2, gy, oz + 2, 4, 4, 4);
        structgen_plan_try_add(plan, PIECE_WELL, fp, STRUCTGEN_NORTH,
                               structgen_seed_mix(site->seed, 0x5e11u));
    }

    // 2. iterate the rest of the grid. each non-center plot rolls for content.
    for (int gz = -GRID_RAD; gz <= GRID_RAD; gz++) {
        for (int gx = -GRID_RAD; gx <= GRID_RAD; gx++) {
            if (gx == 0 && gz == 0) continue;   // well lives here

            float roll = structgen_rng_f01(&rng);
            int ox, oz;
            plot_origin(site, gx, gz, &ox, &oz);

            // pieces face toward the center plot so doors open onto the square.
            structgen_dir face = STRUCTGEN_NORTH;
            if (gx > 0)      face = STRUCTGEN_WEST;
            else if (gx < 0) face = STRUCTGEN_EAST;
            else if (gz > 0) face = STRUCTGEN_NORTH;
            else             face = STRUCTGEN_SOUTH;

            uint32_t pseed = structgen_seed_mix(site->seed,
                                structgen_hash2(gx, gz, site->seed));

            if (roll < 0.50f) {
                // house, randomized footprint inside the plot.
                int w = 5 + structgen_rng_range(&rng, 0, 2);
                int d = 5 + structgen_rng_range(&rng, 0, 2);
                structgen_box fp = structgen_box_at(ox + 1, gy + 1, oz + 1, w, 1, d);
                structgen_plan_try_add(plan, PIECE_HOUSE, fp, face, pseed);
            } else if (roll < 0.75f) {
                // farm fills most of the plot.
                structgen_box fp = structgen_box_at(ox + 1, gy, oz + 1, PLOT - 2, 1, PLOT - 2);
                structgen_plan_try_add(plan, PIECE_FARM, fp, face, pseed);
            }
            // else: empty courtyard plot, skipped.
        }
    }
}

int structgen_village_generate(structgen_buffer *out, const structgen_site *site) {
    structgen_plan plan;
    structgen_village_layout(&plan, site);
    return structgen_plan_build(&plan, out);
}
