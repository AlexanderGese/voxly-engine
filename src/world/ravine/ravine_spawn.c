#include "ravine_spawn.h"

#include "ravine_rand.h"
#include <stddef.h>

int ravine_spawn_region(const ravine_params *p, int region_x, int region_z,
                        ravine_anchor *out) {
    int rarity = p->rarity > 0 ? p->rarity : 1;

    // one roll per region. hash the region coords, modulo rarity.
    uint32_t h = ravine_hash2(region_x, region_z, p->seed ^ 0x12abcdefu);
    if ((h % (uint32_t)rarity) != 0u) return 0;

    // jitter the anchor inside the region so ravines dont land on a visible grid.
    int base_x = region_x * RAVINE_REGION_BLOCKS;
    int base_z = region_z * RAVINE_REGION_BLOCKS;

    float jx = ravine_hash_f01(region_x, region_z * 3 + 1, p->seed ^ 0x771u);
    float jz = ravine_hash_f01(region_x * 7 + 2, region_z, p->seed ^ 0x991u);

    // keep the head off the very edge so most of the ravine fits the region
    // neighbourhood; knot_jitter widens the spread but stays bounded.
    float margin = p->knot_jitter + 4.0f;
    float span = (float)RAVINE_REGION_BLOCKS - 2.0f * margin;
    if (span < 1.0f) span = 1.0f;

    out->region_x = region_x;
    out->region_z = region_z;
    out->anchor_x = (float)base_x + margin + jx * span;
    out->anchor_z = (float)base_z + margin + jz * span;
    out->stream   = ravine_hash1(h, 0xca7a1u);
    return 1;
}

// floor-div rounding toward negative infinity, so regions tile cleanly into
// negative world space. plain int division truncates toward zero, which would
// double-count region 0.
static int floordiv(int a, int b) {
    int q = a / b;
    int r = a % b;
    if ((r != 0) && ((r < 0) != (b < 0))) q--;
    return q;
}

int ravine_spawn_collect(const ravine_params *p, int cx, int cz,
                         ravine_anchor *out, int max) {
    int wx = cx * CHUNK_SIZE_X;
    int wz = cz * CHUNK_SIZE_Z;
    int home_x = floordiv(wx, RAVINE_REGION_BLOCKS);
    int home_z = floordiv(wz, RAVINE_REGION_BLOCKS);

    int n = 0;
    for (int dz = -1; dz <= 1 && n < max; dz++) {
        for (int dx = -1; dx <= 1 && n < max; dx++) {
            ravine_anchor a;
            if (ravine_spawn_region(p, home_x + dx, home_z + dz, &a))
                out[n++] = a;
        }
    }
    return n;
}
