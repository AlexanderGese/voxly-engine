#include "stronghold_corridor.h"
#define COR_HALF   1   // half-width of the air channel (so 3 wide)
#define COR_CEIL   3   // interior height
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
int a_floor = ra->box.y0 + 1;
int b_floor = rb->box.y0 + 1;
int dxs, dzs;
stronghold_dir_step(e->side_a, &dxs, &dzs);
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
int mx = bdx, mz = adz;
int leg2_floor = (e->stair) ? b_floor : a_floor;
n += run_axis(out, mx, mz, bdx - dxs, bdz - dzs, leg2_floor, &n);
n += punch_door(ra, out, adx, a_floor, adz, e->side_a, gated);
n += punch_door(rb, out, bdx, b_floor, bdz, side_b, gated);
return n;
