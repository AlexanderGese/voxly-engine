#include "structgen_piece.h"

// dungeon room + corridor builders. these carve and wall: the room is a hollow
// brick shell with an air interior (a "spawner-ish" pedestal in the middle),
// the corridor is a low brick tube of air linking rooms together.

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

    // brick tube, then air the inside. corridors are 1 wide tall enough to walk.
    n += structgen_buffer_fill_shell(out, fp, BLOCK_BRICK);
    structgen_box inner = structgen_box_inset(fp, 1, 1);
    // a corridor that's only 1 thick on an axis insets to nothing there; the
    // shell pass already covered it, so guard against negative-volume fills.
    if (structgen_box_volume(&inner) > 0)
        n += structgen_buffer_fill_box(out, inner, BLOCK_AIR);

    // punch open the two ends so it actually connects to whatever it abuts.
    int dx, dz;
    structgen_dir_step(pc->facing, &dx, &dz);
    int midx = fp.x0 + structgen_box_width(&fp) / 2;
    int midy = fp.y0 + 1;
    int midz = fp.z0 + structgen_box_depth(&fp) / 2;
    if (dx) {
        n += structgen_buffer_add(out, fp.x0, midy, midz, BLOCK_AIR);
        n += structgen_buffer_add(out, fp.x1 - 1, midy, midz, BLOCK_AIR);
    } else {
        n += structgen_buffer_add(out, midx, midy, fp.z0, BLOCK_AIR);
        n += structgen_buffer_add(out, midx, midy, fp.z1 - 1, BLOCK_AIR);
    }
    return n;
}
