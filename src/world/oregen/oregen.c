#include "oregen.h"
#include <stddef.h>
#include "oregen_seed.h"
#include "oregen_blob.h"
#include "oregen_table.h"

// per-chunk scratch for seeded veins. we bound it so a runaway table cant
// blow the stack. eight ore kinds at a handful of tries each fits easily.
#define OREGEN_MAX_VEINS 64

oregen_params oregen_params_default(uint32_t world_seed) {
    oregen_params p;
    p.world_seed       = world_seed;
    p.max_veins        = OREGEN_MAX_VEINS;
    p.drop_underbudget = 1;
    return p;
}

int oregen_generate_chunk_ex(oregen_buf *buf, int origin_x, int origin_z,
                             int surface_y, const oregen_params *params,
                             int *veins_out) {
    if (veins_out) *veins_out = 0;
    if (!buf) return 0;

    oregen_params local;
    if (params) {
        local = *params;
    } else {
        local = oregen_params_default(0);
    }

    int cap = local.max_veins;
    if (cap <= 0 || cap > OREGEN_MAX_VEINS) cap = OREGEN_MAX_VEINS;

    // dont bother seeding above the actual surface, and never above what the
    // ore table even reaches.
    int max_y = surface_y;
    int table_top = oregen_table_max_y();
    if (max_y > table_top) max_y = table_top;
    if (max_y < oregen_table_min_y()) return 0;   // chunk too shallow for ore

    oregen_vein veins[OREGEN_MAX_VEINS];
    int nv = oregen_seed_chunk(veins, cap, origin_x, origin_z, max_y,
                               local.world_seed);

    int total = 0;
    int contributed = 0;
    for (int i = 0; i < nv; i++) {
        int before = buf->count;
        oregen_blob_build(buf, &veins[i]);
        int got = buf->count - before;

        if (got > 0) {
            contributed++;
            total += got;
        } else if (!local.drop_underbudget) {
            // even an empty roll counts as an attempt if the caller wants
            // the bookkeeping. nothing to add though.
            contributed++;
        }

        if (oregen_buf_full(buf)) break;
    }

    if (veins_out) *veins_out = contributed;
    return total;
}

int oregen_generate_chunk(oregen_buf *buf, int origin_x, int origin_z,
                          int surface_y, const oregen_params *params) {
    return oregen_generate_chunk_ex(buf, origin_x, origin_z, surface_y,
                                    params, NULL);
}
