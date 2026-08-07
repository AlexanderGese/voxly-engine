#include "physics.h"
#include "../world/block.h"
#include "../config.h"

#include <math.h>

// player aabb:
// center x/z = pos.x/z
// half width = PLAYER_WIDTH/2
// y        = [pos.y, pos.y + PLAYER_HEIGHT]

static int block_solid_at(world *w, int x, int y, int z) {
    return block_is_solid(world_get_block(w, x, y, z));
}

static int player_hits(world *w, vec3 pos) {
    float hw = PLAYER_WIDTH * 0.5f;
    int x0 = (int)floorf(pos.x - hw);
    int x1 = (int)floorf(pos.x + hw);
    int z0 = (int)floorf(pos.z - hw);
    int z1 = (int)floorf(pos.z + hw);
    int y0 = (int)floorf(pos.y);
    int y1 = (int)floorf(pos.y + PLAYER_HEIGHT - 0.001f);

    for (int y = y0; y <= y1; y++) {
        for (int z = z0; z <= z1; z++) {
            for (int x = x0; x <= x1; x++) {
                if (block_solid_at(w, x, y, z)) return 1;
            }
        }
    }
    return 0;
}

void physics_move_player(player *p, world *w, float dt) {
    vec3 np = p->pos;

    // x axis
    np.x = p->pos.x + p->vel.x * dt;
    if (player_hits(w, np)) {
        np.x = p->pos.x;
        p->vel.x = 0;
    }
    p->pos.x = np.x;

    // z axis
    np.z = p->pos.z + p->vel.z * dt;
    if (player_hits(w, np)) {
        np.z = p->pos.z;
        p->vel.z = 0;
    }
    p->pos.z = np.z;

    // y axis
    np.y = p->pos.y + p->vel.y * dt;
    if (player_hits(w, np)) {
        if (p->vel.y < 0) p->on_ground = 1;
        np.y = p->pos.y;
        p->vel.y = 0;
    } else {
        p->on_ground = 0;
    }
    p->pos.y = np.y;
}
