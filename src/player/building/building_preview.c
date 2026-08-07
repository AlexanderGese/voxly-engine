#include "building_preview.h"
#include "building_face.h"

void building_preview_init(building_preview *pv) {
    pv->show_outline = 0;
    pv->outline = aabb_make(VEC3_ZERO, VEC3_ZERO);
    pv->outline_face = -1;
    pv->show_ghost = 0;
    pv->ghost = aabb_make(VEC3_ZERO, VEC3_ZERO);
    pv->ghost_id = BLOCK_AIR;
    pv->ghost_valid = 0;
    pv->grace = 0;
}

aabb building_preview_box(int x, int y, int z, float inflate) {
    vec3 mn = vec3_new((float)x - inflate, (float)y - inflate, (float)z - inflate);
    vec3 mx = vec3_new((float)x + 1.0f + inflate,
                       (float)y + 1.0f + inflate,
                       (float)z + 1.0f + inflate);
    return aabb_make(mn, mx);
}

void building_preview_update(building_preview *pv, const building_target *t,
                             block_id place_id, int place_ok, int grace_frames) {
    if (!t || !t->valid) {
        // nothing under the crosshair this frame. let the outline coast on its
        // grace counter so it doesn't blink at block seams.
        if (pv->grace > 0) {
            pv->grace--;
        } else {
            pv->show_outline = 0;
            pv->outline_face = -1;
        }
        pv->show_ghost = 0;
        return;
    }

    // a hair of inflation so the wire box sits just proud of the block surface
    // and doesn't z-fight with the terrain mesh.
    pv->outline = building_preview_box(t->hit_x, t->hit_y, t->hit_z, 0.002f);
    pv->outline_face = t->face;
    pv->show_outline = 1;
    pv->grace = grace_frames;

    if (place_id == BLOCK_AIR) {
        pv->show_ghost = 0;
        return;
    }

    pv->ghost = building_preview_box(t->place_x, t->place_y, t->place_z, 0.0f);
    pv->ghost_id = place_id;
    pv->ghost_valid = (place_ok == 0);   // BPLACE_OK == 0
    pv->show_ghost = 1;
}

void building_preview_corners(aabb box, vec3 out[8]) {
    // corner order: bit 0 = x, bit 1 = y, bit 2 = z. min when bit clear.
    for (int i = 0; i < 8; i++) {
        out[i].x = (i & 1) ? box.max.x : box.min.x;
        out[i].y = (i & 2) ? box.max.y : box.min.y;
        out[i].z = (i & 4) ? box.max.z : box.min.z;
    }
}

void building_preview_edges(int out[24]) {
    // 12 edges of a cube as corner-index pairs, matching the bit layout above.
    // x-aligned edges (toggle bit0), y-aligned (bit1), z-aligned (bit2).
    static const int E[24] = {
        0,1, 2,3, 4,5, 6,7,   // along x
        0,2, 1,3, 4,6, 5,7,   // along y
        0,4, 1,5, 2,6, 3,7,   // along z
    };
    for (int i = 0; i < 24; i++) out[i] = E[i];
}
