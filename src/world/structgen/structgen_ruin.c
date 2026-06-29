#include "structgen_ruin.h"
#include "structgen_rand.h"

void structgen_ruin_layout(structgen_plan *plan, const structgen_site *site) {
    structgen_rng rng;
    structgen_rng_seed(&rng, site->seed ^ 0x52121u);

    int gy = site->ground_y;
    structgen_plan_init(plan, site->anchor_x, gy, site->anchor_z);

    // main hall, sized off the seed. sits one block into the ground so the
    // cracked floor is flush with the surface.
    int w = 7 + structgen_rng_range(&rng, 0, 4);   // 7..11
    int d = 7 + structgen_rng_range(&rng, 0, 4);
    structgen_box main = structgen_box_at(site->anchor_x - w / 2, gy,
                                          site->anchor_z - d / 2, w, 6, d);
    structgen_plan_try_add(plan, PIECE_PILLAR_HALL, main, STRUCTGEN_NORTH,
                           structgen_seed_mix(site->seed, 3u));

    // maybe a satellite hall, offset along a random axis with a road gap.
    if (structgen_rng_chance(&rng, 0.45f)) {
        int sw = 5 + structgen_rng_range(&rng, 0, 2);
        int sd = 5 + structgen_rng_range(&rng, 0, 2);
        structgen_dir side = (structgen_dir)structgen_rng_range(&rng, 0, 3);
        int dx, dz;
        structgen_dir_step(side, &dx, &dz);
        int gap = 3;
        int ox = site->anchor_x + dx * (w / 2 + gap + sw / 2) - sw / 2;
        int oz = site->anchor_z + dz * (d / 2 + gap + sd / 2) - sd / 2;
        structgen_box sat = structgen_box_at(ox, gy, oz, sw, 5, sd);
        structgen_plan_try_add(plan, PIECE_PILLAR_HALL, sat, side,
                               structgen_seed_mix(site->seed, 5u));
    }
}

int structgen_ruin_generate(structgen_buffer *out, const structgen_site *site) {
    structgen_plan plan;
    structgen_ruin_layout(&plan, site);
    return structgen_plan_build(&plan, out);
}
