#include "stronghold_room.h"
static int light_room(const stronghold_room *r, stronghold_buffer *out) {
    int n = 0;
    int fy = r->box.y0 + 1;   // floor + 1
    // four inner corners, one block off the wall.
    int xs[2] = { r->box.x0 + 1, r->box.x1 - 2 };
    int zs[2] = { r->box.z0 + 1, r->box.z1 - 2 };
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            n += stronghold_buffer_add(out, xs[i], fy, zs[j], BLOCK_TORCH);
    return n;
}

// base shell shared by every room type: brick walls/floor/ceiling, hollow air
// inside. cobble-mottled floor for that worn dungeon look.
static int carve_shell(const stronghold_room *r, stronghold_buffer *out,
                       stronghold_rng *rng) {
    int n = 0;
n += stronghold_buffer_fill_shell(out, r->box, BLOCK_BRICK);
stronghold_box inner = stronghold_box_inset(r->box, -1, -1);
n += stronghold_buffer_carve(out, inner);
for (int z = r->box.z0;
z < r->box.z1;
z++)
        for (int x = r->box.x0;
x < r->box.x1;
x++)
            if (stronghold_rng_chance(rng, 0.18f))
                n += stronghold_buffer_add(out, x, r->box.y0, z, BLOCK_COBBLE);
return n;
}

// junction: a 4-way hub gets a stout central pillar so it reads as load-bearing.
static int decorate_junction(const stronghold_room *r, stronghold_buffer *out) {
    int cx, cy, cz;
    stronghold_box_center(&r->box, &cx, &cy, &cz);
    return stronghold_buffer_fill_column(out, cx, r->box.y0 + 1, r->box.y1 - 1, cz, BLOCK_BRICK);
}

// prison: line the long wall with 2-wide barred cells. bars faked with glass.
static int decorate_prison(const stronghold_room *r, stronghold_buffer *out) {
    int n = 0;
int fy = r->box.y0 + 1;
int wide_x = stronghold_box_width(&r->box) >= stronghold_box_depth(&r->box);
int span = wide_x ? stronghold_box_width(&r->box) : stronghold_box_depth(&r->box);
for (int s = 2;
s < span - 2;
s += 3) {
        int cx = wide_x ? r->box.x0 + s : r->box.x1 - 2;
        int cz = wide_x ? r->box.z1 - 2 : r->box.z0 + s;
        // bar front: a glass post with an air gap to look like a cell door.
        for (int dy = 0; dy < 3; dy++) {
            n += stronghold_buffer_add(out, cx, fy + dy, cz, BLOCK_GLASS);
        }
        // open the middle bar so the cell is enterable (it's a ruin).
        n += stronghold_buffer_add(out, cx, fy, cz, BLOCK_AIR);
    }
    return n;
return n;
