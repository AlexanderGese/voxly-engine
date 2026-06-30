#ifndef WORLD_TREEGEN_GRASS_H
#define WORLD_TREEGEN_GRASS_H

#include <stdint.h>
#include "../block.h"

// ground cover scatter: tall grass, flowers, the odd patch. this is a 2d pass
// over a column footprint that decides, per cell, whether a one-block plant sits
// on the surface. it's deterministic on (world xz, seed) so it never flickers
// between reloads and two adjacent chunks agree on the seam.
//
// kept separate from the tree path because it places exactly one block per cell
// and doesn't need the voxel buffer; the deco driver queries it per surface cell.

typedef enum {
    TREEGEN_COVER_NONE = 0,
    TREEGEN_COVER_GRASS,        // tall grass / fern
    TREEGEN_COVER_FLOWER,       // colored bloom
    TREEGEN_COVER_DEADBUSH,     // dry tuft
    TREEGEN_COVER_COUNT
} treegen_cover;

// per-biome scatter weights, 0..255, plus a master density. data only.
typedef struct {
    uint8_t density;            // odds any cover appears at all, 0..255
    uint8_t w_grass;            // relative weights among the cover kinds
    uint8_t w_flower;
    uint8_t w_deadbush;
} treegen_scatter;

treegen_scatter treegen_scatter_default(void);

// decide cover for one surface cell. wx/wz world coords, surface_block is the
// block directly under the candidate cell (cover only sits on grass/dirt/sand).
treegen_cover treegen_grass_pick(int wx, int wz, block_id surface_block,
                                 const treegen_scatter *cfg, uint32_t seed);

// resolve a cover kind to the block id to place. flowers vary their tint by a
// secondary hash so a meadow isn't monochrome (we encode tint in the block id's
// low bits if the atlas has variants; otherwise it's just BLOCK_GRASS-ish).
block_id treegen_cover_block(treegen_cover c, int wx, int wz, uint32_t seed);

// fill a whole CHUNK_SIZE_X by CHUNK_SIZE_Z mask in one shot. `out` is row-major
// [z*CHUNK_SIZE_X + x]; surf is the matching surface-block array. returns how
// many cells got cover. handy so the driver can scatter a chunk without a
// per-cell call overhead.
int treegen_grass_scatter_chunk(int chunk_wx, int chunk_wz,
                                const block_id *surf, treegen_cover *out,
                                const treegen_scatter *cfg, uint32_t seed);

#endif
