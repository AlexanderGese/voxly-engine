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
mineshaft_box room = mineshaft_box_make(cell_box.x0, floor_y, cell_box.z0,
                                            cell_box.x1, ceil_y + 1, cell_box.z1);
n += mineshaft_buffer_carve_room(b, room, cfg->mat_fill, BLOCK_AIR);
int veins = 3 + mineshaft_rng_range(rng, 0, 3);
for (int v = 0;
v < veins;
z < room.z1 - 1;
int chz = room.z0 + 1;
n += mineshaft_buffer_add(b, chx, floor_y + 1, chz, cfg->mat_chest);
return n;
}
