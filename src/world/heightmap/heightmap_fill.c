#include "heightmap_fill.h"
#include "heightmap_strata.h"
#include "../block.h"

#define BEDROCK_THICK  3   // bottom few layers are unbreakable floor

int heightmap_fill_soil_depth(const heightmap_params *p,
                              const heightmap_column *col) {
    (void)p;
    // thin the soil on steep faces so cliffs end up rocky. base of 4 blocks,
    // dropping toward 1 as steepness climbs past ~1.0.
    float s = col->steepness;
    int depth = 4 - (int)(s * 3.0f);
    if (depth < 1) depth = 1;
    if (depth > 4) depth = 4;
    return depth;
}

int heightmap_fill_block(const heightmap_params *p,
                         const heightmap_column *col, int y) {
    int top = col->surface_y;
    int sea = p->sea_level;

    if (y < 0) return BLOCK_AIR;

    // unbreakable floor at the very bottom
    if (y < BEDROCK_THICK) return BLOCK_BEDROCK;

    // above the terrain: water up to sea level, then air
    if (y > top) {
        if (y <= sea) return BLOCK_WATER;
        return BLOCK_AIR;
    }

    // the surface block itself
    if (y == top) {
        return heightmap_column_surface_block(p, col);
    }

    // just under the surface: soil band, unless this column wants bare rock
    int soil = heightmap_fill_soil_depth(p, col);
    if (y > top - soil) {
        int surf = heightmap_column_surface_block(p, col);
        // dont put dirt under sand or under bare stone surfaces
        if (surf == BLOCK_SAND)  return BLOCK_SAND;
        if (surf == BLOCK_STONE) return BLOCK_STONE;
        return BLOCK_DIRT;
    }

    // everything deeper is stone, but let the strata table pick the exact
    // rock so the deep bands arent all identical. it collapses to stone for
    // most bands today; the regolith just under the soil reads as cobble.
    heightmap_stratum st = heightmap_strata_at(p, col, y);
    if (st == HEIGHTMAP_STRATUM_TOPSOIL) return BLOCK_DIRT;
    return heightmap_strata_block(st);
}

int heightmap_fill_range(const heightmap_params *p, const heightmap_column *col,
                         int y0, int y1, uint8_t *out) {
    int solid = 0;
    for (int y = y0; y <= y1; y++) {
        int b = heightmap_fill_block(p, col, y);
        out[y - y0] = (uint8_t)b;
        if (b != BLOCK_AIR) solid++;
    }
    return solid;
}
