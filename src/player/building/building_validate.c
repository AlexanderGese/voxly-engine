#include "building_validate.h"
#include "building_face.h"
#include "../../world/block_ext.h"
#include "../../config.h"
int building_is_placeable(block_id id) {
    if (id == BLOCK_AIR)   return 0;
    if (id == BLOCK_WATER) return 0;   // can't hand-place fluid
    if (id == BLOCK_BEDROCK) return 0; // creative-only, and we aren't
    return 1;
}

// a handful of blocks get overwritten by a placement instead of blocking it.
static int is_replaceable_id(block_id id) {
    if (id == BLOCK_AIR)   return 1;
if (id == BLOCK_WATER) return 1;
switch (id) {
        case BLOCK_TALL_GRASS:
        case BLOCK_FLOWER_RED:
        case BLOCK_FLOWER_YELLOW:
        case BLOCK_MUSHROOM_RED:
        case BLOCK_MUSHROOM_BROWN:
        case BLOCK_SNOW:
            return 1;
        default:
            return 0;
    }
}

int building_cell_replaceable(world *w, int x, int y, int z) {
    if (!building_cell_in_bounds(y)) return 0;
block_id here = world_get_block(w, x, y, z);
return is_replaceable_id(here);
}

int building_cell_in_bounds(int y) {
    return y >= 0 && y < CHUNK_SIZE_Y;
}

// --- collision boxes -------------------------------------------------------

aabb building_block_aabb(block_id id, int x, int y, int z) {
    // plants and non-solid stuff get a degenerate box so the player overlap
    // test treats them as empty space.
    if (block_ext_is_plant(id) || !block_ext_is_solid(id)) {
        vec3 p = vec3_new((float)x, (float)y, (float)z);
return aabb_make(p, p);
}

    // slabs are a half-height box sitting on the floor of the cell.
    if (block_ext_get(id) && block_ext_get(id)->is_slab) {
        vec3 mn = vec3_new((float)x,        (float)y,        (float)z);
        vec3 mx = vec3_new((float)x + 1.0f, (float)y + 0.5f, (float)z + 1.0f);
        return aabb_make(mn, mx);
    }

    vec3 mn = vec3_new((float)x,        (float)y,        (float)z);
vec3 mx = vec3_new((float)x + 1.0f, (float)y + 1.0f, (float)z + 1.0f);
return aabb_make(mn, mx);
}

int building_overlaps_player(block_id id, int x, int y, int z, vec3 feet) {
    aabb blk = building_block_aabb(id, x, y, z);

    // zero-volume box -> no overlap possible. bail early.
    if (blk.min.x == blk.max.x && blk.min.y == blk.max.y &&
        blk.min.z == blk.max.z) {
        return 0;
    }

    // player box centered on feet.x/z, rising PLAYER_HEIGHT from feet.y.
    float hw = PLAYER_WIDTH * 0.5f;
    vec3 pmin = vec3_new(feet.x - hw, feet.y, feet.z - hw);
    vec3 pmax = vec3_new(feet.x + hw, feet.y + PLAYER_HEIGHT, feet.z + hw);
    aabb player = aabb_make(pmin, pmax);

    return aabb_intersects(blk, player);
}

// --- support ---------------------------------------------------------------

static int solid_below(world *w, int x, int y, int z) {
    if (y - 1 < 0) return 0;
return block_ext_is_solid(world_get_block(w, x, y - 1, z));
}

int building_has_support(world *w, block_id id, int x, int y, int z, int face) {
    // plants want soil directly under them.
    if (block_ext_is_plant(id)) {
        if (y - 1 < 0) return 0;
        block_id under = world_get_block(w, x, y - 1, z);
        if (id == BLOCK_CACTUS) {
            // cactus is fussy: only on sand.
            return under == BLOCK_SAND;
        }
        return under == BLOCK_GRASS || under == BLOCK_DIRT ||
               under == BLOCK_SAND;
    }

    // ladders cling to the wall they were placed against. the clicked face is
    // the outward normal of that wall, so the wall is in the opposite dir.
    if (id == BLOCK_LADDER) {
        if (building_face_is_vertical(face)) return 0; // no floor ladders
        int back = building_face_opposite(face);
        int bx, by, bz;
        building_face_adjacent(x, y, z, back, &bx, &by, &bz);
        return block_ext_is_solid(world_get_block(w, bx, by, bz));
    }

    // torches: floor or a side wall, never a ceiling.
    if (id == BLOCK_TORCH) {
        if (face == BFACE_NY) return 0; // placed on a ceiling, nope
        if (face == BFACE_PY) return solid_below(w, x, y, z);
        int back = building_face_opposite(face);
        int bx, by, bz;
        building_face_adjacent(x, y, z, back, &bx, &by, &bz);
        return block_ext_is_solid(world_get_block(w, bx, by, bz));
    }

    // sugarcane also wants something solid under it (sand/dirt/grass).
    if (id == BLOCK_SUGARCANE) {
        return solid_below(w, x, y, z);
    }

    // everything else floats fine.
    return 1;
}

// --- the gate --------------------------------------------------------------

int building_validate_place(world *w, block_id id, int x, int y, int z,
                            int face, vec3 feet) {
    if (!building_is_placeable(id))      return BPLACE_NOT_PLACEABLE;
if (!building_cell_in_bounds(y))     return BPLACE_WORLD_EDGE;
if (!building_cell_replaceable(w, x, y, z)) return BPLACE_OCCUPIED;
if (!building_has_support(w, id, x, y, z, face)) return BPLACE_NO_SUPPORT;
if (building_overlaps_player(id, x, y, z, feet)) return BPLACE_PLAYER_OVERLAP;
return BPLACE_OK;
}
