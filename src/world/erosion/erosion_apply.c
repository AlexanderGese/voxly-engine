#include "erosion_apply.h"

#include "../../config.h"
#include <math.h>

block_id erosion_deposit_block(int world_y, int sea_level) {
    // material the water/scree drops. sand collects in the wet lowlands, dirt
    // on the slopes, and the very top of a deposit gets grass so it blends.
    if (world_y <= sea_level + 1) return BLOCK_SAND;
    return BLOCK_DIRT;
}

// is this block one the erosion stage is allowed to remove? we only wash away
// soft surface material, never carve into stone/bedrock or drain water.
static int erodible(block_id b) {
    return b == BLOCK_GRASS || b == BLOCK_DIRT ||
           b == BLOCK_SAND  || b == BLOCK_SNOW;
}

int erosion_apply_column(chunk *c, int lx, int lz,
                         int old_top, int new_top, int sea_level) {
    int changed = 0;

    if (new_top < old_top) {
        // erosion lowered the surface. clear soft blocks from old_top down to
        // new_top+1, but stop early if we hit stone so we never gouge rock.
        for (int y = old_top; y > new_top; y--) {
            if (y < 0 || y >= CHUNK_SIZE_Y) continue;
            block_id b = chunk_get_block(c, lx, y, lz);
            if (!erodible(b)) break;
            chunk_set_block(c, lx, y, lz, BLOCK_AIR);
            changed--;
        }
        // re-cap the new surface with grass so we dont leave bare dirt.
        if (new_top >= 0 && new_top < CHUNK_SIZE_Y) {
            block_id top = chunk_get_block(c, lx, new_top, lz);
            if (top == BLOCK_DIRT && new_top > sea_level + 1)
                chunk_set_block(c, lx, new_top, lz, BLOCK_GRASS);
        }
    } else if (new_top > old_top) {
        // deposition raised the surface. fill the gap with sediment material.
        // demote the old grass cap to dirt first, its buried now.
        if (old_top >= 0 && old_top < CHUNK_SIZE_Y) {
            if (chunk_get_block(c, lx, old_top, lz) == BLOCK_GRASS)
                chunk_set_block(c, lx, old_top, lz, BLOCK_DIRT);
        }
        for (int y = old_top + 1; y <= new_top; y++) {
            if (y < 0 || y >= CHUNK_SIZE_Y) continue;
            block_id fill = (y == new_top)
                          ? (y > sea_level + 1 ? BLOCK_GRASS
                                               : erosion_deposit_block(y, sea_level))
                          : erosion_deposit_block(y, sea_level);
            chunk_set_block(c, lx, y, lz, fill);
            changed++;
        }
    }
    return changed;
}

int erosion_apply_chunk(chunk *c, const erosion_field *f,
                        const int *old_height, int sea_level) {
    int total = 0;
    for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
            // chunk-local maps to the field interior past the pad.
            int fx = lx + EROSION_PAD;
            int fz = lz + EROSION_PAD;
            int oi = fz * EROSION_DIM_X + fx;

            float eroded = f->height[erosion_idx(fx, fz)] + f->sediment[erosion_idx(fx, fz)];
            int new_top = (int)floorf(eroded + 0.5f);
            int old_top = old_height[oi];

            if (new_top < 0) new_top = 0;
            if (new_top > CHUNK_SIZE_Y - 1) new_top = CHUNK_SIZE_Y - 1;

            total += erosion_apply_column(c, lx, lz, old_top, new_top, sea_level);
        }
    }
    return total;
}
