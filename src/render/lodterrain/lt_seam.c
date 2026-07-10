#include "lt_seam.h"

// face -> neighbour slot. only the four horizontal faces map; top/bottom never
// border another chunk (the world is one chunk tall) so they return -1.
int lt_seam_dir(int face) {
    switch (face) {
    case LT_FACE_PX: return 0;
    case LT_FACE_NX: return 1;
    case LT_FACE_PZ: return 2;
    case LT_FACE_NZ: return 3;
    default:         return -1;
    }
}

// the seam bit for each slot, in the same px/nx/pz/nz order.
static const lt_seam_mask SLOT_BIT[4] = {
    LT_SEAM_PX, LT_SEAM_NX, LT_SEAM_PZ, LT_SEAM_NZ
};

lt_seam_mask lt_seam_compute(int my_level, const lt_neighbor_levels *nb) {
    lt_seam_mask mask = 0;
    for (int s = 0; s < 4; s++) {
        int nl = nb->level[s];
        if (nl < 0) continue;            // no chunk there, nothing to crack
        if (nl > my_level) mask |= SLOT_BIT[s];   // neighbour coarser -> skirt
    }
    return mask;
}

int lt_seam_has(lt_seam_mask mask, int face) {
    int slot = lt_seam_dir(face);
    if (slot < 0) return 0;
    return (mask & SLOT_BIT[slot]) != 0;
}
