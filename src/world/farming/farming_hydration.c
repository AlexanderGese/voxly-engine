#include "farming_hydration.h"
#include "../block.h"
#include "../../config.h"

// seconds of dryness before we knock a hydration point off. watering resets the
// timer, so a tile next to water never ticks down.
#define DRY_STEP_SECONDS   8.0f

int farming_hydration_water_near(world *w, int wx, int wy, int wz) {
    const int r = FARMING_WATER_RADIUS;
    // chebyshev box on xz, +-1 on y. classic 9x3x9-ish neighborhood, minus the
    // tile itself which is farmland not water.
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -r; dz <= r; dz++) {
            for (int dx = -r; dx <= r; dx++) {
                if (dx == 0 && dy == 0 && dz == 0) continue;
                block_id b = world_get_block(w, wx + dx, wy + dy, wz + dz);
                if (b == BLOCK_WATER) return 1;
            }
        }
    }
    return 0;
}

int farming_hydration_sky_open(world *w, int wx, int wy, int wz) {
    // walk up; if we hit any opaque block before the build ceiling, the tile is
    // covered and rain cant reach it.
    for (int y = wy + 1; y < CHUNK_SIZE_Y; y++) {
        block_id b = world_get_block(w, wx, y, wz);
        if (b != BLOCK_AIR && block_is_opaque(b)) return 0;
    }
    return 1;
}

void farming_hydration_step(farming_tile *tile, int water_near, int rained,
                            float dt) {
    if (water_near || rained) {
        // wet: refill toward max and forget about drying.
        tile->dry_timer = 0.0f;
        if (tile->hydration < FARMING_HYDRATION_MAX) {
            // top up a touch faster than it dries so a brief water gap recovers.
            tile->dry_timer -= 0.0f; // (no-op, keep timer at floor)
            tile->hydration++;
        }
        return;
    }

    // dry: accumulate time, drop a point each DRY_STEP_SECONDS.
    tile->dry_timer += dt;
    while (tile->dry_timer >= DRY_STEP_SECONDS && tile->hydration > 0) {
        tile->dry_timer -= DRY_STEP_SECONDS;
        tile->hydration--;
    }
    // when already at zero dont let the timer run away unbounded; the tile
    // machine reads it to decide on reverting, and it only cares up to a cap.
    if (tile->hydration == 0 && tile->dry_timer > DRY_STEP_SECONDS * 4.0f)
        tile->dry_timer = DRY_STEP_SECONDS * 4.0f;
}

float farming_hydration_factor(const farming_tile *tile) {
    // 0..7 -> 0.35..1.0. even dry land grows, just at a third pace, which is
    // enough to be annoying but not a softlock.
    float h = (float)tile->hydration / (float)FARMING_HYDRATION_MAX;
    return 0.35f + 0.65f * h;
}
