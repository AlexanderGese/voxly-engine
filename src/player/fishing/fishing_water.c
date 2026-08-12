#include "fishing_water.h"
#include "../../config.h"
#include <limits.h>

int fishing_water_is_water(world *w, int wx, int wy, int wz) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return 0;
    return world_get_block(w, wx, wy, wz) == BLOCK_WATER;
}

int fishing_water_surface_y(world *w, int wx, int wz, int *found) {
    // scan down from a bit above sea level. the first water cell whose top
    // neighbour is non-water is the surface. cheap and robust enough.
    int top = WORLD_SEA_LEVEL + 8;
    if (top >= CHUNK_SIZE_Y) top = CHUNK_SIZE_Y - 1;

    for (int y = top; y >= 0; y--) {
        if (fishing_water_is_water(w, wx, y, wz)) {
            if (found) *found = 1;
            return y;
        }
    }
    if (found) *found = 0;
    return INT_MIN;
}

// count water cells in the 3x3 ring (excluding centre) at this y. a full ring
// of 8 means we're out in open water; a low count means we're near a shore.
static int ring_water_count(world *w, int wx, int wy, int wz) {
    int n = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dz = -1; dz <= 1; dz++) {
            if (dx == 0 && dz == 0) continue;
            if (fishing_water_is_water(w, wx + dx, wy, wz + dz)) n++;
        }
    }
    return n;
}

// is there clear sky straight up from the surface. roofed-over water counts as
// obstructed even if the surface itself is wide open.
static int has_sky_access(world *w, int wx, int wy, int wz) {
    for (int y = wy + 1; y < CHUNK_SIZE_Y; y++) {
        block_id b = world_get_block(w, wx, y, wz);
        if (b == BLOCK_AIR || b == BLOCK_WATER) continue;
        return 0;   // hit something solid before the sky
    }
    return 1;
}

fishing_water_kind fishing_water_classify(world *w, int wx, int wy, int wz) {
    if (!fishing_water_is_water(w, wx, wy, wz)) return WATER_NONE;

    int ring = ring_water_count(w, wx, wy, wz);
    int sky  = has_sky_access(w, wx, wy, wz);

    // open needs a mostly-complete ring and a view of the sky. otherwise it's
    // still fishable, just penalised.
    if (ring >= 7 && sky) return WATER_OPEN;
    return WATER_OBSTRUCTED;
}

float fishing_water_quality(world *w, int wx, int wy, int wz) {
    if (!fishing_water_is_water(w, wx, wy, wz)) return 0.0f;

    int ring = ring_water_count(w, wx, wy, wz);   // 0..8
    float openness = (float)ring / 8.0f;

    // sky access is worth a flat bump; without it the spot tops out around 0.6.
    float sky = has_sky_access(w, wx, wy, wz) ? 1.0f : 0.6f;

    float q = openness * sky;
    if (q < 0.05f) q = 0.05f;     // never fully dead if there's any water
    if (q > 1.0f)  q = 1.0f;
    return q;
}
