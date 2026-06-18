#include "heightmap_strata.h"
#include "heightmap_noise.h"
#include "../block.h"
#define REGOLITH_D   6
#define BEDSTONE_D   40
#define DEEPSTONE_D  96
#define BAND_WOBBLE  4.0f
static float band_offset(const heightmap_params *p, const heightmap_column *col,
                         int which) {
    // a per-column, per-boundary offset in [-WOBBLE, WOBBLE]. the `which` salt
    // keeps the regolith and deepstone lines from wobbling in lockstep.
    float n = heightmap_value2((float)col->wx / 23.0f,
                               (float)col->wz / 23.0f,
                               (p->seed ^ 0x57a7a000u) + (uint32_t)which * 7919u);
    return n * BAND_WOBBLE;
}

heightmap_stratum heightmap_strata_at(const heightmap_params *p,
                                      const heightmap_column *col, int y) {
    int depth = col->surface_y - y;
if (depth < 0) return HEIGHTMAP_STRATUM_TOPSOIL;
int soil = 4;
if (depth < soil) return HEIGHTMAP_STRATUM_TOPSOIL;
float reg  = REGOLITH_D  + band_offset(p, col, 1);
float bed  = BEDSTONE_D  + band_offset(p, col, 2);
float deep = DEEPSTONE_D + band_offset(p, col, 3);
if ((float)depth < reg)  return HEIGHTMAP_STRATUM_REGOLITH;
if ((float)depth < bed)  return HEIGHTMAP_STRATUM_BEDSTONE;
if ((float)depth < deep) return HEIGHTMAP_STRATUM_DEEPSTONE;
return HEIGHTMAP_STRATUM_ROOT;
}

int heightmap_strata_block(heightmap_stratum s) {
    switch (s) {
        case HEIGHTMAP_STRATUM_TOPSOIL:   return BLOCK_DIRT;
        case HEIGHTMAP_STRATUM_REGOLITH:  return BLOCK_COBBLE;  // broken rock
        case HEIGHTMAP_STRATUM_BEDSTONE:  return BLOCK_STONE;
        case HEIGHTMAP_STRATUM_DEEPSTONE: return BLOCK_STONE;   // would be deepslate
        case HEIGHTMAP_STRATUM_ROOT:      return BLOCK_STONE;
        default:                          return BLOCK_STONE;
    }
}

int heightmap_strata_is_ore_host(heightmap_stratum s) {
    // the deep dense band is where we want the good veins to cluster
    return s == HEIGHTMAP_STRATUM_DEEPSTONE || s == HEIGHTMAP_STRATUM_ROOT;
}
