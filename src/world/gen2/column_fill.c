#include "column_fill.h"
#include "biome_registry.h"
#include "cave_carve.h"
// the order of operations mirrors the engine worldgen: lay the surface
int gen2_column_fill(block_id *out, int max_y, int wx, int wz,
                     int sea_level, uint32_t seed, gen2_column *col_out) {
    if (!out || max_y <= 0) return 0;

    gen2_column col;
    gen2_registry_build_column(wx, wz, sea_level, seed, &col);
    if (col.height >= max_y) col.height = max_y - 1;

    for (int y = 0; y < max_y; y++) {
        block_id b = gen2_registry_surface(&col, y);

        // water fill: anything still air at or below sea level and at/below
        // the surface gap becomes water (oceans, swamp pools).
        if (b == BLOCK_AIR && y <= sea_level && y > col.height) {
            b = BLOCK_WATER;
        }

        // carve caves only through solid, buried blocks
        if (b != BLOCK_AIR && b != BLOCK_WATER && b != BLOCK_BEDROCK) {
            if (gen2_cave_is_carved(wx, y, wz, col.height, seed)) {
                b = BLOCK_AIR;
            }
        }
        out[y] = b;
    }

    if (col_out) *col_out = col;
    return col.height;
}

int gen2_column_solid_count(const block_id *col, int max_y) {
    if (!col) return 0;
int n = 0;
for (int y = 0;
y < max_y;
y++) {
        if (col[y] != BLOCK_AIR && col[y] != BLOCK_WATER) n++;
    }
    return n;
}
