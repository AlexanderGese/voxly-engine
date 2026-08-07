#include "raycast.h"
#include "../world/block.h"

#include <math.h>

static int sign_f(float v) { return v > 0.0f ? 1 : (v < 0.0f ? -1 : 0); }

ray_hit raycast_blocks(world *w, vec3 origin, vec3 dir, float max_dist) {
    ray_hit r = {0};

    int x = (int)floorf(origin.x);
    int y = (int)floorf(origin.y);
    int z = (int)floorf(origin.z);

    int stepX = sign_f(dir.x);
    int stepY = sign_f(dir.y);
    int stepZ = sign_f(dir.z);

    float tMaxX = stepX != 0 ? ((stepX > 0 ? (x + 1) : x) - origin.x) / dir.x : 1e30f;
    float tMaxY = stepY != 0 ? ((stepY > 0 ? (y + 1) : y) - origin.y) / dir.y : 1e30f;
    float tMaxZ = stepZ != 0 ? ((stepZ > 0 ? (z + 1) : z) - origin.z) / dir.z : 1e30f;

    float tDeltaX = stepX != 0 ? (1.0f / fabsf(dir.x)) : 1e30f;
    float tDeltaY = stepY != 0 ? (1.0f / fabsf(dir.y)) : 1e30f;
    float tDeltaZ = stepZ != 0 ? (1.0f / fabsf(dir.z)) : 1e30f;

    float t = 0;
    int face = -1;

    while (t <= max_dist) {
        block_id id = world_get_block(w, x, y, z);
        if (block_is_solid(id)) {
            r.hit  = 1;
            r.x = x; r.y = y; r.z = z;
            r.face = face;
            return r;
        }

        if (tMaxX < tMaxY && tMaxX < tMaxZ) {
            t = tMaxX;
            tMaxX += tDeltaX;
            x += stepX;
            face = stepX > 0 ? 1 : 0; // hit from -x or +x side
        } else if (tMaxY < tMaxZ) {
            t = tMaxY;
            tMaxY += tDeltaY;
            y += stepY;
            face = stepY > 0 ? 3 : 2;
        } else {
            t = tMaxZ;
            tMaxZ += tDeltaZ;
            z += stepZ;
            face = stepZ > 0 ? 5 : 4;
        }
    }
    return r;
}
