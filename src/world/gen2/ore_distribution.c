#include "ore_distribution.h"
#include "gen2_noise.h"

// classic minecraft-ish bands: common stuff spread everywhere, rare stuff
// hugs the bottom. blocks are all cobble for now (we lack ore textures).

static const gen2_ore_kind g_ores[] = {
    { "coal",     5, 90, 14, 4, 9, BLOCK_COBBLE },
    { "iron",     5, 60,  9, 3, 7, BLOCK_COBBLE },
    { "gold",     4, 30,  3, 2, 5, BLOCK_COBBLE },
    { "diamond",  2, 16,  1, 2, 6, BLOCK_COBBLE },
};
#define ORE_COUNT ((int)(sizeof(g_ores) / sizeof(g_ores[0])))

int gen2_ore_kind_count(void) { return ORE_COUNT; }

const gen2_ore_kind *gen2_ore_kind_at(int i) {
    if (i < 0 || i >= ORE_COUNT) return &g_ores[0];
    return &g_ores[i];
}

int gen2_ore_allowed_at_y(int k, int y) {
    if (k < 0 || k >= ORE_COUNT) return 0;
    return (y >= g_ores[k].y_min && y <= g_ores[k].y_max);
}

// emit a blobby vein around (cx,cy,cz). walks a short random path.
static int emit_vein(gen2_place_buf *out, int cx, int cy, int cz,
                     int size, block_id block, uint32_t seed) {
    int n = 0;
    int x = cx, y = cy, z = cz;
    for (int i = 0; i < size; i++) {
        n += gen2_place_add(out, x, y, z, block);
        uint32_t h = voxl_gen2_hash3(x, y, z, seed + (uint32_t)i * 131u);
        // step one axis by +/-1
        switch (h % 6u) {
            case 0: x++; break;
            case 1: x--; break;
            case 2: y++; break;
            case 3: y--; break;
            case 4: z++; break;
            default: z--; break;
        }
    }
    return n;
}

int gen2_ore_generate(gen2_place_buf *out, int origin_x, int origin_z,
                      int region_w, int region_d, int max_y, uint32_t seed) {
    if (region_w <= 0 || region_d <= 0) return 0;
    int n = 0;

    for (int k = 0; k < ORE_COUNT; k++) {
        const gen2_ore_kind *ore = &g_ores[k];
        uint32_t ks = seed ^ ((uint32_t)k * 0x9e3779b9u);

        for (int a = 0; a < ore->per_chunk; a++) {
            uint32_t h = voxl_gen2_hash3(origin_x, a, origin_z, ks + (uint32_t)a);
            int lx = (int)(h % (uint32_t)region_w);
            h = voxl_gen2_hash2((int)h, a, ks);
            int lz = (int)(h % (uint32_t)region_d);

            int ymin = ore->y_min;
            int ymax = ore->y_max < max_y ? ore->y_max : max_y;
            if (ymax <= ymin) continue;
            int y = ymin + (int)(h % (uint32_t)(ymax - ymin));

            int size = ore->vein_min;
            int span = ore->vein_max - ore->vein_min;
            if (span > 0) size += (int)(h % (uint32_t)(span + 1));

            n += emit_vein(out, origin_x + lx, y, origin_z + lz,
                           size, ore->block, ks + 7u * (uint32_t)a);
        }
    }
    return n;
}
