#include "farming_query.h"
#include "farming_block.h"
#include "../block.h"

block_id farming_query_below(world *w, int wx, int wy, int wz) {
    return world_get_block(w, wx, wy - 1, wz);
}

block_id farming_query_above(world *w, int wx, int wy, int wz) {
    return world_get_block(w, wx, wy + 1, wz);
}

int farming_query_crop_supported(world *w, int wx, int wy, int wz) {
    block_id below = farming_query_below(w, wx, wy, wz);
    if (!farming_block_is_farmland(below)) return 0;
    block_id here = world_get_block(w, wx, wy, wz);
    // either still our crop block, or air (mid-replant). anything solid that
    // isnt ours means something stomped the plant; treat as unsupported.
    return here == BLOCK_AIR || farming_block_is_crop(here);
}

int farming_query_plantable(world *w, int wx, int wy, int wz) {
    if (!farming_block_is_farmland(farming_query_below(w, wx, wy, wz))) return 0;
    return world_get_block(w, wx, wy, wz) == BLOCK_AIR;
}

int farming_query_free_fruit_spots(world *w, int wx, int wy, int wz,
                                   int *out_dx, int *out_dz) {
    // the four cardinals. a fruit needs air at the spot and a tillable/solid
    // block beneath it to rest on (so it doesnt spawn floating).
    static const int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    int n = 0;
    for (int i = 0; i < 4; i++) {
        int nx = wx + dirs[i][0];
        int nz = wz + dirs[i][1];
        if (world_get_block(w, nx, wy, nz) != BLOCK_AIR) continue;
        block_id ground = world_get_block(w, nx, wy - 1, nz);
        if (ground == BLOCK_AIR || !block_is_solid(ground)) {
            // farmland is solid-ish for our purposes; let it host fruit too.
            if (!farming_block_is_farmland(ground)) continue;
        }
        out_dx[n] = dirs[i][0];
        out_dz[n] = dirs[i][1];
        n++;
    }
    return n;
}

int farming_query_light(world *w, int wx, int wy, int wz) {
    int sun = world_get_sunlight(w, wx, wy, wz);
    int blk = world_get_blocklight(w, wx, wy, wz);
    return sun > blk ? sun : blk;
}
