#ifndef WORLD_HEIGHTMAP_STRATA_H
#define WORLD_HEIGHTMAP_STRATA_H

#include <stdint.h>
#include "heightmap_params.h"
#include "heightmap_column.h"

// depth-banded rock stratification for the subsurface. the fill stage decides
// the broad bands (soil / stone / bedrock); this refines the stone band into
// named strata so the world isnt one grey block all the way down. we dont have
// many actual block ids to spend, so most strata collapse to stone or cobble,
// but the band math is the real artefact and it drops in cleanly the day there
// are deepslate / tuff / andesite ids to map onto.
//
// strata are measured as depth below the column surface, not absolute y, so a
// mountain and a plain share the same rock sequence under their feet. a little
// per-column noise wobbles the band boundaries so they dont read as perfectly
// flat geological lines, which always looks fake.

typedef enum {
    HEIGHTMAP_STRATUM_TOPSOIL = 0,  // the soil band, handled upstream
    HEIGHTMAP_STRATUM_REGOLITH,     // broken-up near-surface rock
    HEIGHTMAP_STRATUM_BEDSTONE,     // the main stone body
    HEIGHTMAP_STRATUM_DEEPSTONE,    // dense deep rock, ore-rich band
    HEIGHTMAP_STRATUM_ROOT,         // just above bedrock
    HEIGHTMAP_STRATUM_COUNT
} heightmap_stratum;

// which stratum sits at world y in this column. pure depth + noise, no block
// decision. callers map the result to whatever block they have.
heightmap_stratum heightmap_strata_at(const heightmap_params *p,
                                      const heightmap_column *col, int y);

// map a stratum to the block id we actually place. collapses onto the small
// block set we have today. returns a block_id-compatible int.
int heightmap_strata_block(heightmap_stratum s);

// 1 if this stratum is a band ore generators should prefer (the deep, dense
// rock). the oregen driver can use it to bias vein placement downward without
// hardcoding a y threshold.
int heightmap_strata_is_ore_host(heightmap_stratum s);

#endif
