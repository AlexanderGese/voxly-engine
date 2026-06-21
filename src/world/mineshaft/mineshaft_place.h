#ifndef WORLD_MINESHAFT_PLACE_H
#define WORLD_MINESHAFT_PLACE_H

#include <stdint.h>
#include "mineshaft_types.h"

// placement: decide whether a placement region hosts a mineshaft and, if so,
// where its anchor sits. shafts are rare and spaced on a coarse grid so they
// don't pile up. one region = cfg->region_size x region_size chunks; each rolls
// once. fully deterministic on (region coords, world seed).

// resolve the region containing chunk (ccx,ccz) to a site. returns 1 and fills
// *out (anchor xz + derived seed; floor_y left for the driver to resolve from
// the heightmap) if the region hosts a shaft, else 0.
int mineshaft_pick(const mineshaft_config *cfg, int ccx, int ccz,
                   uint32_t world_seed, mineshaft_site *out);

#endif
