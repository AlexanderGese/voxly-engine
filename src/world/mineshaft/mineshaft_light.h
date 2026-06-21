#ifndef WORLD_MINESHAFT_LIGHT_H
#define WORLD_MINESHAFT_LIGHT_H

#include "mineshaft_buffer.h"
#include "mineshaft_box.h"
#include "mineshaft_grid.h"
#include "mineshaft_rand.h"
#include "mineshaft_types.h"

// sparse lighting. the miners hung torches at intervals; most have guttered out
// by now, so we place them thinly and bias them toward junctions and shaft
// landings where you'd want light most. torch id is cfg->mat_torch. the engine's
// own lighting pass propagates the glow once these blocks land - we just stamp
// the emitters. placement is deterministic-per-position so it doesn't flicker on
// regen, with the rng only deciding the overall survival rate.

// scatter a few wall torches around a cell's perimeter at corridor head height.
// `survival` in [0,1] is the chance any candidate slot still holds a lit torch.
// returns voxels added.
int mineshaft_light_cell(mineshaft_buffer *b, const mineshaft_config *cfg,
                         mineshaft_box cell_box, int floor_y, int ceil_y,
                         float survival, uint32_t seed);

// torches survive better near important cells. derive a per-cell survival rate
// from its kind so junctions stay lit while plain corridors go dark.
float mineshaft_light_survival(const mineshaft_grid *g, int cx, int cz);

#endif
