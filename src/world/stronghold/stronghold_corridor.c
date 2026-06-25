#include "stronghold_corridor.h"

// corridors are 3 wide, 3 tall on the inside, wrapped in a stone-brick shell.
// we carve the air channel and lay the shell as we go. the doorway is a 1x2
// gap in the room wall, sealed back to air so the player can walk through.
#define COR_HALF   1   // half-width of the air channel (so 3 wide)
#define COR_CEIL   3   // interior height

// stamp a single cross-section of corridor at (x,*,z), axis-aligned to the
// run direction. lays floor+ceiling+side walls in stone brick, air inside.
static int section(stronghold_buffer *out, int x, int floor_y, int z,
                   int along_x, int *count) {
    int n = 0;
    // perpendicular axis: if we run along x, the walls are at z+/-2.
    for (int dy = -1; dy <= COR_CEIL; dy++) {
        for (int w = -COR_HALF - 1; w <= COR_HALF + 1; w++) {
            int wx = along_x ? x : x + w;
            int wz = along_x ? z + w : z;
            int y = floor_y + dy;
            int edge_w = (w == -COR_HALF - 1 || w == COR_HALF + 1);
            int edge_y = (dy == -1 || dy == COR_CEIL);
            if (edge_w || edge_y) {
                n += stronghold_buffer_add(out, wx, y, wz, BLOCK_BRICK);
            } else {
                n += stronghold_buffer_add(out, wx, y, wz, BLOCK_AIR);
            }
        }
    }
    *count += n;
    return n;
}

// punch a 1x2 doorway in a room wall at the cell the corridor meets, facing
// `facing` (the dir pointing out of the room). returns voxels touched.
static int punch_door(stronghold_room *r, stronghold_buffer *out,
                      int x, int floor_y, int z, stronghold_dir facing, int gated) {
    int n = 0;
    n += stronghold_buffer_add(out, x, floor_y,     z, BLOCK_AIR);
    n += stronghold_buffer_add(out, x, floor_y + 1, z, BLOCK_AIR);
    // lintel above stays solid; record the door so the decorator can dress it.
    stronghold_room_add_door(r, x, floor_y, z, facing, gated);
    return n;
}

// straight run from (x0,z0) to (x1,z1) along one axis. exactly one of dx/dz is
// nonzero. floor stays at floor_y for the whole run.
static int run_axis(stronghold_buffer *out, int x0, int z0, int x1, int z1,
                    int floor_y, int *count) {
    int along_x = (z0 == z1);
    int n = 0;
    if (along_x) {
        int step = x1 >= x0 ? 1 : -1;
        for (int x = x0; x != x1 + step; x += step)
            section(out, x, floor_y, z0, 1, &n);
    } else {
        int step = z1 >= z0 ? 1 : -1;
        for (int z = z0; z != z1 + step; z += step)
            section(out, x0, floor_y, z, 0, &n);
    }
    *count += n;
    return n;
}

int stronghold_corridor_carve(stronghold_graph *g, int edge_idx, stronghold_buffer *out) {
    stronghold_edge *e = &g->edges[edge_idx];
    stronghold_room *ra = &g->rooms[e->a];
    stronghold_room *rb = &g->rooms[e->b];

    int acx, acy, acz, bcx, bcy, bcz;
    stronghold_box_center(&ra->box, &acx, &acy, &acz);
    stronghold_box_center(&rb->box, &bcx, &bcy, &bcz);

    // door cells sit on the room walls along the connecting axis. floor heights
    // come from each room's own floor (rooms may be on different levels).
    int a_floor = ra->box.y0 + 1;
    int b_floor = rb->box.y0 + 1;

    int dxs, dzs;
    stronghold_dir_step(e->side_a, &dxs, &dzs);

    // doorway cell on room a: one block past a's wall in side_a.
    int adx = (e->side_a == STRONGHOLD_WEST)  ? ra->box.x0
            : (e->side_a == STRONGHOLD_EAST)  ? ra->box.x1 - 1 : acx;
    int adz = (e->side_a == STRONGHOLD_NORTH) ? ra->box.z0
            : (e->side_a == STRONGHOLD_SOUTH) ? ra->box.z1 - 1 : acz;

    stronghold_dir side_b = stronghold_dir_opposite(e->side_a);
    int bdx = (side_b == STRONGHOLD_WEST)  ? rb->box.x0
            : (side_b == STRONGHOLD_EAST)  ? rb->box.x1 - 1 : bcx;
    int bdz = (side_b == STRONGHOLD_NORTH) ? rb->box.z0
            : (side_b == STRONGHOLD_SOUTH) ? rb->box.z1 - 1 : bcz;

    int n = 0;

    // first corridor leg leaves room a in side_a, out to the midpoint, then the
    // perpendicular leg lines up with room b. classic L-bend. we run at a's
    // floor until the bend, then if it's a stair edge we splice a shaft.
    int mx = bdx, mz = adz;        // bend point: travel x to b, keep a's z first
    if (dzs != 0) { mx = adx; mz = bdz; }   // if a left on z, do z then x

    // leg 1: a's door cell to the bend.
    n += run_axis(out, adx + dxs, adz + dzs, mx, mz, a_floor, &n);
    // leg 2: bend to b's door cell, at b's floor.
    int leg2_floor = (e->stair) ? b_floor : a_floor;
    n += run_axis(out, mx, mz, bdx - dxs, bdz - dzs, leg2_floor, &n);

    // for stair edges, drop a vertical shaft at the bend joining the two floors.
    if (e->stair && a_floor != b_floor) {
        int lo = a_floor < b_floor ? a_floor : b_floor;
        int hi = a_floor < b_floor ? b_floor : a_floor;
        for (int y = lo - 1; y <= hi + COR_CEIL; y++) {
            for (int dz = -COR_HALF - 1; dz <= COR_HALF + 1; dz++)
                for (int dx = -COR_HALF - 1; dx <= COR_HALF + 1; dx++) {
                    int edge = (dx == -COR_HALF - 1 || dx == COR_HALF + 1 ||
                                dz == -COR_HALF - 1 || dz == COR_HALF + 1);
                    block_id id = edge ? BLOCK_BRICK : BLOCK_AIR;
                    n += stronghold_buffer_add(out, mx + dx, y, mz + dz, id);
                }
        }
        // ladder of torches climbing the shaft so it reads as a stairwell.
        for (int y = lo; y <= hi; y += 2)
            n += stronghold_buffer_add(out, mx, y, mz, BLOCK_TORCH);
    }

    // doorways at both rooms. prison/portal edges get gated doors.
    int gated = (ra->type == STRONGHOLD_ROOM_PRISON || rb->type == STRONGHOLD_ROOM_PRISON);
    n += punch_door(ra, out, adx, a_floor, adz, e->side_a, gated);
    n += punch_door(rb, out, bdx, b_floor, bdz, side_b, gated);

    return n;
}

int stronghold_corridor_carve_all(stronghold_graph *g, stronghold_buffer *out) {
    int n = 0;
    for (int e = 0; e < g->edge_count; e++)
        n += stronghold_corridor_carve(g, e, out);
    return n;
}
