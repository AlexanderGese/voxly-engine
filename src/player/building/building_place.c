#include "building_place.h"
#include "building_validate.h"
#include "building_rotation.h"

#include "../../config.h"

void building_mark_dirty(world *w, int wx, int wy, int wz) {
    int cx, cz, lx, lz;
    world_to_chunk(wx, wz, &cx, &cz);
    world_to_local(wx, wz, &lx, &lz);

    chunk *c = world_get_chunk(w, cx, cz);
    if (c) c->dirty = 1;

    // if the edit touches a chunk edge, the neighbor's boundary faces need to
    // be rebuilt too. check each of the four horizontal borders.
    if (lx == 0) { chunk *n = world_get_chunk(w, cx - 1, cz); if (n) n->dirty = 1; }
    if (lx == CHUNK_SIZE_X - 1) { chunk *n = world_get_chunk(w, cx + 1, cz); if (n) n->dirty = 1; }
    if (lz == 0) { chunk *n = world_get_chunk(w, cx, cz - 1); if (n) n->dirty = 1; }
    if (lz == CHUNK_SIZE_Z - 1) { chunk *n = world_get_chunk(w, cx, cz + 1); if (n) n->dirty = 1; }

    (void)wy; // vertical neighbors live in the same chunk column, no-op.
}

int building_place_block(world *w, building_history *hist,
                         block_id id, const building_target *t,
                         float yaw, vec3 player_feet,
                         building_edit *out_edit) {
    if (id == BLOCK_AIR)        return BPLACE_EMPTY_HAND;
    if (!t || !t->valid)        return BPLACE_NO_TARGET;

    int x = t->place_x, y = t->place_y, z = t->place_z;

    // run the full rule gate first; cheap to fail early.
    int verdict = building_validate_place(w, id, x, y, z, t->face, player_feet);
    if (verdict != BPLACE_OK) return verdict;

    // resolve the concrete (possibly rotated) variant to actually store.
    block_id stored = building_rotation_resolve(id, t->face, yaw);

    block_id before = world_get_block(w, x, y, z);
    world_set_block(w, x, y, z, stored);
    building_mark_dirty(w, x, y, z);

    building_edit e = { x, y, z, before, stored };
    if (hist) building_history_record(hist, &e);
    if (out_edit) *out_edit = e;

    return BPLACE_OK;
}
