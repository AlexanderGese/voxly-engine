#include "spawn.h"
#include "../world/worldgen.h"
#include "../world/block.h"
#include "../config.h"

vec3 spawn_find(world *w, int wx, int wz) {
    // scan a small area for the highest non-water block
    int best_y = -1;
    int best_wx = wx, best_wz = wz;
    for (int dz = -4; dz <= 4; dz++) {
        for (int dx = -4; dx <= 4; dx++) {
            int x = wx + dx;
            int z = wz + dz;
            int h = worldgen_height_at(x, z, w->seed);
            if (h > best_y && h >= WORLD_SEA_LEVEL) {
                best_y = h;
                best_wx = x;
                best_wz = z;
            }
        }
    }
    if (best_y < 0) best_y = WORLD_SEA_LEVEL + 2;
    return (vec3){ best_wx + 0.5f, best_y + 1.5f, best_wz + 0.5f };
}
