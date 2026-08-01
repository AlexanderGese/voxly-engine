#include "mspawn_ground.h"
#include "../../config.h"
#include "../../world/block.h"

// clamp helper, the usual.
static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

int mspawn_find_floor(world *w, int wx, int wz, int y_hint, int *out_y) {
    // walk down from the hint looking for the first solid block that has a
    // non-solid block directly above it. that pairing is what "a surface" means
    // here; a solid under solid is buried and no good.
    int top = clampi(y_hint, 1, CHUNK_SIZE_Y - 2);
    for (int y = top; y >= 1; y--) {
        block_id here  = world_get_block(w, wx, y, wz);
        block_id above = world_get_block(w, wx, y + 1, wz);
        if (block_is_solid(here) && !block_is_solid(above)) {
            *out_y = y;
            return 1;
        }
    }
    *out_y = -1;
    return 0;
}

int mspawn_light_at(world *w, int wx, int wy_floor, int wz) {
    int wy = wy_floor + 1;          // feet sit one above the floor
    if (wy >= CHUNK_SIZE_Y) wy = CHUNK_SIZE_Y - 1;
    int bl = world_get_blocklight(w, wx, wy, wz);
    int sl = world_get_sunlight(w, wx, wy, wz);
    return bl > sl ? bl : sl;
}

// is there a clear column of `need` non-solid blocks above the floor?
static int has_headroom(world *w, int wx, int wy_floor, int wz, int need) {
    for (int i = 1; i <= need; i++) {
        int wy = wy_floor + i;
        if (wy >= CHUNK_SIZE_Y) return 0;
        if (block_is_solid(world_get_block(w, wx, wy, wz))) return 0;
    }
    return 1;
}

int mspawn_site_ok(world *w, const mspawn_entry *e, int wx, int wy_floor,
                   int wz, uint32_t seed, mspawn_site *out) {
    // y band. -1 sentinels mean "no constraint" on that side.
    if (e->y_min >= 0 && wy_floor < e->y_min) return 0;
    if (e->y_max >= 0 && wy_floor > e->y_max) return 0;

    block_id floor = world_get_block(w, wx, wy_floor, wz);
    block_id over  = world_get_block(w, wx, wy_floor + 1, wz);

    switch (e->ground) {
        case MSPAWN_GROUND_SOLID:
            // standard case: solid opaque floor, clear air above.
            if (!block_is_solid(floor)) return 0;
            if (floor == BLOCK_WATER || over == BLOCK_WATER) return 0;
            if (!has_headroom(w, wx, wy_floor, wz, MSPAWN_HEADROOM)) return 0;
            break;
        case MSPAWN_GROUND_WATER:
            // wants the feet block to be water. floor can be anything below.
            if (over != BLOCK_WATER) return 0;
            break;
        case MSPAWN_GROUND_AIR:
            // cave dweller: just needs the headroom, floor merely solid-ish.
            if (!has_headroom(w, wx, wy_floor, wz, MSPAWN_HEADROOM)) return 0;
            break;
        default:
            return 0;
    }

    int light = mspawn_light_at(w, wx, wy_floor, wz);
    if (light < e->min_light || light > e->max_light) return 0;

    out->pos   = vec3_new((float)wx + 0.5f, (float)(wy_floor + 1), (float)wz + 0.5f);
    out->type  = e->type;
    out->light = light;
    out->wy    = wy_floor;
    out->seed  = seed;
    return 1;
}

int mspawn_try_site(world *w, const mspawn_entry *e, int cx, int cz,
                    int radius, int y_hint, mspawn_rng *r, mspawn_site *out) {
    // roll an offset inside the ring. rejection-sample a disc so we dont bias
    // the corners; cheap because radius is small.
    int ox, oz;
    int guard = 0;
    do {
        ox = mspawn_rng_range(r, -radius, radius);
        oz = mspawn_rng_range(r, -radius, radius);
    } while (ox * ox + oz * oz > radius * radius && ++guard < 8);

    int wx = cx + ox;
    int wz = cz + oz;

    int wy;
    if (!mspawn_find_floor(w, wx, wz, y_hint, &wy)) return 0;

    uint32_t seed = mspawn_hash2(wx, wz, (uint32_t)(wy * 2654435761u));
    return mspawn_site_ok(w, e, wx, wy, wz, seed, out);
}
