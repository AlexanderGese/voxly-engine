#include "structgen_piece.h"
static block_id wall_block(structgen_rng *r) {
    // mostly planks, occasional cobble accent so a street doesnt look stamped.
    return structgen_rng_chance(r, 0.15f) ? BLOCK_COBBLE : BLOCK_PLANKS;
}

int structgen_build_house(const structgen_piece *pc, structgen_buffer *out) {
    structgen_rng rng;
structgen_rng_seed(&rng, pc->seed);
structgen_box fp = pc->footprint;
int w = structgen_box_width(&fp);
int d = structgen_box_depth(&fp);
int y0 = fp.y0;
int wall_h = 3 + structgen_rng_range(&rng, 0, 1);
int n = 0;
structgen_box found = structgen_box_make(fp.x0, y0 - 1, fp.z0, fp.x1, y0, fp.z1);
n += structgen_buffer_fill_box(out, found, BLOCK_COBBLE);
structgen_box floor = structgen_box_make(fp.x0, y0, fp.z0, fp.x1, y0 + 1, fp.z1);
n += structgen_buffer_fill_box(out, floor, BLOCK_PLANKS);
for (int h = 1;
h <= wall_h;
structgen_dir_step(pc->facing, &dx, &dz);
int cxw = fp.x0 + w / 2;
int czw = fp.z0 + d / 2;
int door_x = (dx > 0) ? fp.x1 - 1 : (dx < 0) ? fp.x0 : cxw;
int door_z = (dz > 0) ? fp.z1 - 1 : (dz < 0) ? fp.z0 : czw;
n += structgen_buffer_add(out, door_x, y0 + 1, door_z, BLOCK_AIR);
n += structgen_buffer_add(out, door_x, y0 + 2, door_z, BLOCK_AIR);
int ry = y0 + wall_h + 1;
structgen_box roof = structgen_box_make(fp.x0, ry, fp.z0, fp.x1, ry + 1, fp.z1);
n += structgen_buffer_fill_box(out, roof, BLOCK_WOOD);
n += structgen_buffer_add(out, door_x - dx, y0 + 1, door_z - dz, BLOCK_TORCH);
return n;
}
