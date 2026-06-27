#include "stronghold_portal.h"

// the frame is a 5x5 outline of bricks (one block of moat inside the room),
// sitting over a recessed pit of water (our lava stand-in). a 3-step stair
// leads down from the room floor into the pit lip. torches ring the frame so
// it reads as "active".

// carve the recessed pit and lay the water moat at the bottom.
static int carve_pit(const stronghold_room *r, stronghold_buffer *out,
                     int px0, int pz0, int psize, int pit_y) {
    int n = 0;
    stronghold_box pit = stronghold_box_make(px0, pit_y, pz0,
                                             px0 + psize, r->box.y0 + 1, pz0 + psize);
    n += stronghold_buffer_carve(out, pit);
    // water floor at the very bottom.
    for (int z = px0; z < px0 + psize; z++)
        for (int x = pz0; x < pz0 + psize; x++)
            n += stronghold_buffer_add(out, x, pit_y, z, BLOCK_WATER);
    // brick rim around the pit lip so the floor doesnt look chewed.
    return n;
}

// the 5x5 portal frame outline at floor level, ringed with torches.
static int frame_ring(const stronghold_room *r, stronghold_buffer *out,
                      int fx0, int fz0, int fsize, int fy) {
    int n = 0;
    stronghold_box ring = stronghold_box_at(fx0, fy, fz0, fsize, 1, fsize);
    n += stronghold_buffer_fill_frame(out, ring, fy, BLOCK_BRICK);
    // torch on each frame corner, one block up, so it glows.
    int corners[4][2] = {
        { fx0,            fz0 },
        { fx0 + fsize - 1, fz0 },
        { fx0,            fz0 + fsize - 1 },
        { fx0 + fsize - 1, fz0 + fsize - 1 },
    };
    for (int i = 0; i < 4; i++)
        n += stronghold_buffer_add(out, corners[i][0], fy + 1, corners[i][1], BLOCK_TORCH);
    (void)r;
    return n;
}

int stronghold_portal_carve(const stronghold_room *r, stronghold_buffer *out,
                            stronghold_rng *rng) {
    int n = 0;
    // shell + hollow interior, cobble floor for the worn temple look.
    n += stronghold_buffer_fill_shell(out, r->box, BLOCK_BRICK);
    stronghold_box inner = stronghold_box_inset(r->box, -1, -1);
    n += stronghold_buffer_carve(out, inner);
    for (int z = r->box.z0 + 1; z < r->box.z1 - 1; z++)
        for (int x = r->box.x0 + 1; x < r->box.x1 - 1; x++)
            n += stronghold_buffer_add(out, x, r->box.y0, z, BLOCK_COBBLE);

    int cx, cy, cz;
    stronghold_box_center(&r->box, &cx, &cy, &cz);

    // pit: centered 5x5, recessed 2 below floor.
    int psize = 5;
    int px0 = cx - psize / 2;
    int pz0 = cz - psize / 2;
    int pit_y = r->box.y0 - 2;
    n += carve_pit(r, out, px0, pz0, psize, pit_y);

    // frame sits one block out from the pit, on the room floor.
    int fsize = psize + 2;
    int fx0 = cx - fsize / 2;
    int fz0 = cz - fsize / 2;
    n += frame_ring(r, out, fx0, fz0, fsize, r->box.y0);

    // stairs: a 3-wide flight from the room floor down to the pit lip on the
    // -z side, so the player can descend to the frame.
    for (int s = 0; s < 3; s++) {
        int sy = r->box.y0 - s;
        int sz = pz0 - 1 - s;
        for (int sx = cx - 1; sx <= cx + 1; sx++) {
            n += stronghold_buffer_add(out, sx, sy, sz, BLOCK_COBBLE);
            n += stronghold_buffer_add(out, sx, sy + 1, sz, BLOCK_AIR);
            n += stronghold_buffer_add(out, sx, sy + 2, sz, BLOCK_AIR);
        }
    }

    // spawner stand-in: a lone cobble pedestal with a torch in a back corner.
    int spx = r->box.x0 + 1, spz = r->box.z0 + 1;
    n += stronghold_buffer_add(out, spx, r->box.y0 + 1, spz, BLOCK_COBBLE);
    n += stronghold_buffer_add(out, spx, r->box.y0 + 2, spz, BLOCK_TORCH);

    // a couple of wall sconces for ambience.
    int sconces = stronghold_rng_range(rng, 2, 3);
    for (int i = 0; i < sconces; i++) {
        int wx = stronghold_rng_range(rng, r->box.x0 + 1, r->box.x1 - 2);
        n += stronghold_buffer_add(out, wx, r->box.y1 - 2, r->box.z0 + 1, BLOCK_TORCH);
    }
    return n;
}

int stronghold_portal_build(const stronghold_graph *g, stronghold_buffer *out,
                            stronghold_rng *rng) {
    if (g->portal_room < 0 || g->portal_room >= g->room_count) return 0;
    return stronghold_portal_carve(&g->rooms[g->portal_room], out, rng);
}
