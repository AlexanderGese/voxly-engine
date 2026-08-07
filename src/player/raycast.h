#ifndef PLAYER_RAYCAST_H
#define PLAYER_RAYCAST_H

#include "../math/vec3.h"
#include "../world/world.h"
#include "../world/block.h"

typedef struct {
    int  hit;
    int  x, y, z;
    int  face;   // 0=+x 1=-x 2=+y 3=-y 4=+z 5=-z (face hit, to place against)
} ray_hit;

// DDA voxel ray march. `max_dist` is in block units.
ray_hit raycast_blocks(world *w, vec3 origin, vec3 dir, float max_dist);

#endif
