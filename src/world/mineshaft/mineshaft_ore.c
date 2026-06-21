#include "mineshaft_ore.h"

int mineshaft_ore_vein(mineshaft_buffer *b, int x, int y, int z, int size,
                       block_id ore, mineshaft_rng *rng) {
    // a drunkard's walk dropping an ore block each step. keeps veins blobby and
    // connected rather than a scatter of isolated specks.
    int n = 0;
    int cx = x, cy = y, cz = z;
    for (int i = 0; i < size; i++) {
        n += mineshaft_buffer_add(b, cx, cy, cz, ore);
        int axis = mineshaft_rng_range(rng, 0, 2);
        int step = mineshaft_rng_chance(rng, 0.5f) ? 1 : -1;
        if      (axis == 0) cx += step;
        else if (axis == 1) cy += step;
        else                cz += step;
    }
    return n;
}

int mineshaft_ore_room(mineshaft_buffer *b, const mineshaft_config *cfg,
                       mineshaft_box cell_box, int floor_y, int ceil_y,
                       mineshaft_rng *rng, uint32_t seed) {
    int n = 0;

    // the chamber: a stone-walled room a touch taller than a corridor.
    mineshaft_box room = mineshaft_box_make(cell_box.x0, floor_y, cell_box.z0,
                                            cell_box.x1, ceil_y + 1, cell_box.z1);
    n += mineshaft_buffer_carve_room(b, room, cfg->mat_fill, BLOCK_AIR);

    // expose a few ore veins. anchor each on a random wall cell so the room
    // reads as half-mined-out. richer ore (ore_rare) shows up sparingly.
    int veins = 3 + mineshaft_rng_range(rng, 0, 3);
    for (int v = 0; v < veins; v++) {
        int wall = mineshaft_rng_range(rng, 0, 3);   // -x,+x,-z,+z
        int wx, wz;
        int wy = floor_y + 1 + mineshaft_rng_range(rng, 0, ceil_y - floor_y - 1);
        if (wall == 0)      { wx = room.x0;     wz = mineshaft_rng_range(rng, room.z0 + 1, room.z1 - 2); }
        else if (wall == 1) { wx = room.x1 - 1; wz = mineshaft_rng_range(rng, room.z0 + 1, room.z1 - 2); }
        else if (wall == 2) { wz = room.z0;     wx = mineshaft_rng_range(rng, room.x0 + 1, room.x1 - 2); }
        else                { wz = room.z1 - 1; wx = mineshaft_rng_range(rng, room.x0 + 1, room.x1 - 2); }

        block_id ore = mineshaft_rng_chance(rng, 0.25f) ? cfg->ore_rare
                                                        : cfg->ore_common;
        int size = 4 + mineshaft_rng_range(rng, 0, 5);
        n += mineshaft_ore_vein(b, wx, wy, wz, size, ore, rng);
    }

    // tailings: a low scatter of rubble across the floor, stable per-block.
    for (int z = room.z0 + 1; z < room.z1 - 1; z++) {
        for (int x = room.x0 + 1; x < room.x1 - 1; x++) {
            float r = mineshaft_hash_f01(x, floor_y, z, seed ^ 0x0babe000u);
            if (r < 0.22f)
                n += mineshaft_buffer_add(b, x, floor_y + 1, z, cfg->mat_fill);
        }
    }

    // a loot chest tucked against a wall. one per room, deterministic spot.
    int chx = room.x0 + 1 + (int)(mineshaft_hash2(room.x0, room.z0, seed) % 2);
    int chz = room.z0 + 1;
    n += mineshaft_buffer_add(b, chx, floor_y + 1, chz, cfg->mat_chest);

    return n;
}
