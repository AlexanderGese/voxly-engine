#ifndef RENDER_LODTERRAIN_LT_SEAM_H
#define RENDER_LODTERRAIN_LT_SEAM_H
#include "lt_types.h"
// seam analysis. when two adjacent chunks mesh at different lod levels the
// border cells don't line up and you get cracks — daylight straight through the
// terrain. this module figures out *which* borders are mismatched so the skirt
// pass knows where to drop fill. we don't do true t-junction welding (it's a
// pain across the streaming boundary); skirts are the pragmatic fix everybody
// ships.
// the four horizontal neighbour levels around a chunk, in face order
// px, nx, pz, nz. -1 means "no chunk there / not loaded" — treated as same
// level so we don't skirt against the void.
typedef struct {
    int level[4];   // indexed by lt_seam_dir()
} lt_neighbor_levels;
// map a horizontal LT_FACE_* to the 0..3 slot in lt_neighbor_levels. returns
int lt_seam_dir(int face);
lt_seam_mask lt_seam_compute(int my_level, const lt_neighbor_levels *nb);
int lt_seam_has(lt_seam_mask mask, int face);
#endif
