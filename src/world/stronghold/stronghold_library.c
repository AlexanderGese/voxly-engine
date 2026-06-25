#include "stronghold_library.h"
// a library is at least 2 floors of headroom; if the room is tall enough we cut
// a mid-level walkway. shelves are wood blocks stacked 2-3 high against walls.
#define SHELF_BLOCK   BLOCK_WOOD
#define FLOOR_BLOCK   BLOCK_PLANKS
// line all four interior walls with shelving, leaving the doorway rows clear.
// shelves are 1 deep, run from floor+1 up to a capped height.
static int line_shelves(const stronghold_room *r, stronghold_buffer *out, int top_y) {
    int n = 0;
    int y0 = r->box.y0 + 1;
    int y1 = top_y;            // exclusive
    // inner ring, one block in from the brick wall.
    int x0 = r->box.x0 + 1, x1 = r->box.x1 - 1;
    int z0 = r->box.z0 + 1, z1 = r->box.z1 - 1;
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            // north/south walls
            n += stronghold_buffer_add(out, x, y, z0, SHELF_BLOCK);
            n += stronghold_buffer_add(out, x, y, z1 - 1, SHELF_BLOCK);
        }
        for (int z = z0; z < z1; z++) {
            // east/west walls
            n += stronghold_buffer_add(out, x0, y, z, SHELF_BLOCK);
            n += stronghold_buffer_add(out, x1 - 1, y, z, SHELF_BLOCK);
        }
    }
    return n;
}

// free-standing stacks: short island shelves down the center aisle, 2 high,
// spaced so there's a walkable gap between each.
static int center_stacks(const stronghold_room *r, stronghold_buffer *out) {
    int n = 0;
int cx, cy, cz;
stronghold_box_center(&r->box, &cx, &cy, &cz);
int fy = r->box.y0 + 1;
int span = stronghold_box_depth(&r->box);
for (int s = r->box.z0 + 3;
s < r->box.z1 - 3;
s += 3) {
        n += stronghold_buffer_add(out, cx, fy,     s, SHELF_BLOCK);
        n += stronghold_buffer_add(out, cx, fy + 1, s, SHELF_BLOCK);
    }
    (void)span;
return n;
}

// mid walkway: a plank floor at mid height around the room perimeter, with a
// 1-wide gap on one side so the lower floor stays reachable. only for tall rms.
static int upper_walkway(const stronghold_room *r, stronghold_buffer *out, int mid_y) {
    int n = 0;
    // perimeter walkway, 1 deep, leaving a central void.
    for (int x = r->box.x0 + 1; x < r->box.x1 - 1; x++) {
        n += stronghold_buffer_add(out, x, mid_y, r->box.z0 + 1, FLOOR_BLOCK);
        n += stronghold_buffer_add(out, x, mid_y, r->box.z1 - 2, FLOOR_BLOCK);
    }
    for (int z = r->box.z0 + 1; z < r->box.z1 - 1; z++) {
        n += stronghold_buffer_add(out, r->box.x0 + 1, mid_y, z, FLOOR_BLOCK);
        n += stronghold_buffer_add(out, r->box.x1 - 2, mid_y, z, FLOOR_BLOCK);
    }
    // ladder column up a corner (torches as the climbable fake-ladder).
    int lx = r->box.x0 + 1, lz = r->box.z0 + 1;
    for (int y = r->box.y0 + 1; y <= mid_y; y += 2)
        n += stronghold_buffer_add(out, lx, y, lz, BLOCK_TORCH);
    // clear a hole in the walkway by the ladder so you can climb through.
    n += stronghold_buffer_add(out, lx, mid_y, lz, BLOCK_AIR);
    return n;
}

int stronghold_library_carve(const stronghold_room *r, stronghold_buffer *out,
                             stronghold_rng *rng) {
    int n = 0;
// shell + hollow, like any room but with a plank floor.
n += stronghold_buffer_fill_shell(out, r->box, BLOCK_BRICK);
stronghold_box inner = stronghold_box_inset(r->box, -1, -1);
n += stronghold_buffer_carve(out, inner);
for (int z = r->box.z0 + 1;
z < r->box.z1 - 1;
z++)
        for (int x = r->box.x0 + 1;
x < r->box.x1 - 1;
x++)
            n += stronghold_buffer_add(out, x, r->box.y0, z, FLOOR_BLOCK);
int height = stronghold_box_height(&r->box);
int tall = height >= 7;
int top_y = r->box.y1 - 1;
int mid_y = r->box.y0 + height / 2;
if (tall) {
        // two tiers of shelves split by the walkway.
        n += line_shelves(r, out, mid_y);
        n += upper_walkway(r, out, mid_y);
        n += line_shelves(r, out, top_y);   // upper tier shelves above walkway
    } else {
        n += line_shelves(r, out, top_y - 1);
}

    n += center_stacks(r, out);
// lighting: torches on the center stacks and one per corner.
int cx, cy, cz;
stronghold_box_center(&r->box, &cx, &cy, &cz);
n += stronghold_buffer_add(out, cx, r->box.y1 - 2, cz, BLOCK_TORCH);
// a couple of random wall sconces so big libraries arent gloomy.
int sconces = stronghold_rng_range(rng, 2, 4);
for (int i = 0;
i < sconces;
i++) {
        int sx = stronghold_rng_range(rng, r->box.x0 + 1, r->box.x1 - 2);
        int sz = stronghold_rng_range(rng, r->box.z0 + 1, r->box.z1 - 2);
        n += stronghold_buffer_add(out, sx, r->box.y1 - 2, sz, BLOCK_TORCH);
    }
    return n;
}

int stronghold_library_carve_all(const stronghold_graph *g, stronghold_buffer *out) {
    int n = 0;
    for (int i = 0; i < g->room_count; i++) {
        if (g->rooms[i].type != STRONGHOLD_ROOM_LIBRARY) continue;
        stronghold_rng sub;
        stronghold_rng_seed(&sub, stronghold_seed_mix(g->rooms[i].seed, 0x11b));
        n += stronghold_library_carve(&g->rooms[i], out, &sub);
    }
    return n;
}
