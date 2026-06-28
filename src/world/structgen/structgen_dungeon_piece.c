#include "structgen_piece.h"
int structgen_build_room(const structgen_piece *pc, structgen_buffer *out) {
    structgen_rng rng;
    structgen_rng_seed(&rng, pc->seed);

    structgen_box fp = pc->footprint;
    int n = 0;

    // shell first: brick all six faces.
    n += structgen_buffer_fill_shell(out, fp, BLOCK_BRICK);

    // hollow the interior to air so anything terrain dumped in here is cleared.
    structgen_box inner = structgen_box_inset(fp, 1, 1);
    n += structgen_buffer_fill_box(out, inner, BLOCK_AIR);

    // mossy patches: swap some floor bricks to cobble for that damp look.
    int fy = fp.y0;
    for (int z = fp.z0 + 1; z < fp.z1 - 1; z++)
        for (int x = fp.x0 + 1; x < fp.x1 - 1; x++)
            if (structgen_rng_chance(&rng, 0.25f))
                n += structgen_buffer_add(out, x, fy, z, BLOCK_COBBLE);

    // a central pedestal, one block, where loot/spawner would sit.
    int cx = fp.x0 + structgen_box_width(&fp) / 2;
    int cz = fp.z0 + structgen_box_depth(&fp) / 2;
    n += structgen_buffer_add(out, cx, fp.y0 + 1, cz, BLOCK_COBBLE);

    // wall torches at the corners of the interior ceiling height.
    int ty = fp.y1 - 2;
    n += structgen_buffer_add(out, fp.x0 + 1, ty, fp.z0 + 1, BLOCK_TORCH);
    n += structgen_buffer_add(out, fp.x1 - 2, ty, fp.z1 - 2, BLOCK_TORCH);

    return n;
}

int structgen_build_corridor(const structgen_piece *pc, structgen_buffer *out) {
    structgen_box fp = pc->footprint;
int n = 0;
n += structgen_buffer_fill_shell(out, fp, BLOCK_BRICK);
structgen_box inner = structgen_box_inset(fp, 1, 1);
if (structgen_box_volume(&inner) > 0)
        n += structgen_buffer_fill_box(out, inner, BLOCK_AIR);
int dx, dz;
structgen_dir_step(pc->facing, &dx, &dz);
int midx = fp.x0 + structgen_box_width(&fp) / 2;
int midy = fp.y0 + 1;
int midz = fp.z0 + structgen_box_depth(&fp) / 2;
n += structgen_buffer_add(out, midx, midy, fp.z1 - 1, BLOCK_AIR);
}
    return n;
}
