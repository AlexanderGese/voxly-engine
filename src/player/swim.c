#include "swim.h"
#include "../world/block.h"
#include "../input/input.h"
#include "../config.h"

#include <GLFW/glfw3.h>
#include <math.h>

int swim_is_in_water(const player *p, world *w) {
    int x = (int)floorf(p->pos.x);
    int y = (int)floorf(p->pos.y + PLAYER_EYE_HEIGHT);
    int z = (int)floorf(p->pos.z);
    return world_get_block(w, x, y, z) == BLOCK_WATER;
}

void swim_apply(player *p, world *w, float dt) {
    if (!swim_is_in_water(p, w)) return;
    // damping
    p->vel.x *= 0.85f;
    p->vel.y *= 0.85f;
    p->vel.z *= 0.85f;
    // buoyancy
    p->vel.y += 4.0f * dt;
    if (input_key_held(GLFW_KEY_SPACE)) p->vel.y += 5.0f * dt;
}
