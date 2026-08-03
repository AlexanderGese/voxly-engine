#include "spawnx_commit.h"
#include "../../world/block.h"
#include <math.h>

// world is only 0..CHUNK_SIZE_Y-1 tall. clamp probes so we never read OOB y.
static int y_in_range(int y) {
    return y >= 0 && y < CHUNK_SIZE_Y;
}

// does this voxel block a standing mob? water doesnt count as floor or wall for
// our purposes (water mobs go through a different path), only proper solids do.
static int solid_at(world *w, int wx, int wy, int wz) {
    if (!y_in_range(wy)) return 0;
    block_id b = world_get_block(w, wx, wy, wz);
    return block_is_solid(b) && b != BLOCK_WATER;
}

int spawnx_spot_clear(world *w, int wx, int wy, int wz, int needs_floor) {
    if (!y_in_range(wy)) return 0;

    // feet + headroom must be clear of solids.
    for (int h = 0; h < SPAWNX_HEADROOM; h++) {
        if (solid_at(w, wx, wy + h, wz)) return 0;
    }

    // floor: the block directly under the feet must hold weight.
    if (needs_floor) {
        if (!solid_at(w, wx, wy - 1, wz)) return 0;
    }
    return 1;
}

int spawnx_settle_y(world *w, int wx, int y_hint, int wz, int needs_floor) {
    // search a small band around the hint, nearest-first. up to 4 each way is
    // plenty; the hint comes from the generated surface so it's usually dead on.
    for (int d = 0; d <= 4; d++) {
        int yd = y_hint - d;
        if (spawnx_spot_clear(w, wx, yd, wz, needs_floor)) return yd;
        if (d == 0) continue;
        int yu = y_hint + d;
        if (spawnx_spot_clear(w, wx, yu, wz, needs_floor)) return yu;
    }
    return -1;
}

// is any live mob already standing in this voxel? cheap linear scan, MAX_MOBS
// is tiny. keeps two spawns in one tick from landing on the same tile.
static int voxel_taken(const mob_registry *mr, int wx, int wy, int wz) {
    for (int i = 0; i < mr->count; i++) {
        const entity *e = &mr->list[i];
        if (!e->alive) continue;
        if ((int)floorf(e->pos.x) == wx &&
            (int)floorf(e->pos.y) == wy &&
            (int)floorf(e->pos.z) == wz)
            return 1;
    }
    return 0;
}

int spawnx_commit(world *w, mob_registry *mr, const spawnx_request *req,
                  spawnx_result *out) {
    out->placed = 0;
    out->entity_id = 0;
    out->pos = req->pos;

    int wx = (int)floorf(req->pos.x);
    int wz = (int)floorf(req->pos.z);
    int y_hint = (int)floorf(req->pos.y);

    // command spawns trust the caller's y exactly; everything else settles onto
    // the nearest legal floor so a slightly-off request still lands.
    int wy;
    if (req->source == SPAWNX_SRC_COMMAND) {
        wy = spawnx_spot_clear(w, wx, y_hint, wz, 1) ? y_hint : -1;
    } else {
        wy = spawnx_settle_y(w, wx, y_hint, wz, 1);
    }
    if (wy < 0) return 0;

    if (voxel_taken(mr, wx, wy, wz)) return 0;

    // voxel-centered feet. +0.5 in x/z to sit in the middle of the tile.
    vec3 feet = vec3_new(wx + 0.5f, (float)wy, wz + 0.5f);

    int id = mob_spawn(mr, req->type, feet);
    if (id <= 0) return 0;            // registry full

    out->placed = 1;
    out->entity_id = (uint32_t)id;
    out->pos = feet;
    return 1;
}
