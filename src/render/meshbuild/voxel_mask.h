#ifndef RENDER_MESHBUILD_VOXEL_MASK_H
#define RENDER_MESHBUILD_VOXEL_MASK_H

// the per-slice mask. greedy meshing works one plane at a time: for every cell
// on the plane we decide whether it produces a face, and stash everything
// needed to merge identical faces (tile, light, ao). the merge pass then walks
// the mask growing rectangles.

#include "mb_types.h"

// one mask cell. `present` 0 means "no face here" (air, or hidden by neighbour).
// the rest only matter when present. ao/light are stored so two cells only
// merge if they shade identically — a quad spanning a lit/shadow boundary
// would look wrong if we smeared one corner over the whole thing.
typedef struct {
    uint8_t present;
    uint8_t tile;
    uint8_t block;
    uint8_t ao[4];
    uint8_t light[4];
} mb_cell;

// a slice mask sized for the largest chunk plane. dims (w x h) are set per
// slice. stored row-major: cell(u,v) = cells[v*w + u].
typedef struct {
    int     w, h;
    mb_cell cells[MB_MASK_MAX_DIM * MB_MASK_MAX_DIM];
} mb_mask;

static inline mb_cell *mb_mask_at(mb_mask *m, int u, int v) {
    return &m->cells[v * m->w + u];
}

// wipe a mask to all-empty and set its dimensions.
void mb_mask_reset(mb_mask *m, int w, int h);

// true if two cells describe the exact same surface and can join into one quad.
// compares block/tile and every ao+light corner.
int  mb_cell_eq(const mb_cell *a, const mb_cell *b);

#endif
