#include "ravine_carve.h"

#include "ravine_types.h"

int ravine_carve_is_diggable(block_id id) {
    // the soft stuff the surface is made of. leave ores/bedrock/builds alone —
    // same policy the cave carver uses, so the two passes coexist cleanly.
    switch (id) {
        case BLOCK_STONE:
        case BLOCK_DIRT:
        case BLOCK_GRASS:
        case BLOCK_SAND:
        case BLOCK_SNOW:
        case BLOCK_COBBLE:
            return 1;
        default:
            return 0;
    }
}

ravine_carve_stats ravine_carve_apply(chunk *c, const ravine_field *f,
                                      const ravine_strata *strata,
                                      const ravine_params *p) {
    ravine_carve_stats st = {0, 0, 0, 0};

    for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
            int fx = lx + RAVINE_PAD;
            int fz = lz + RAVINE_PAD;
            int idx = ravine_cell_index(fx, fz);
            if (idx < 0) continue;

            if (f->cut[idx] <= 0) continue;        // column untouched
            int surf  = f->surface[idx];
            int floor = f->floor_y[idx];
            if (floor >= surf) continue;

            uint8_t kind = f->kind[idx];

            int wx, wz;
            ravine_field_to_world(f, fx, fz, &wx, &wz);

            // clear the column from the surface down to (not into) the floor.
            int top = surf;
            if (top > CHUNK_SIZE_Y - 1) top = CHUNK_SIZE_Y - 1;
            for (int y = top; y > floor; y--) {
                block_id cur = chunk_get_block(c, lx, y, lz);
                if (cur == BLOCK_BEDROCK) { st.skipped_bedrock++; continue; }
                if (!ravine_carve_is_diggable(cur)) continue;
                chunk_set_block(c, lx, y, lz, BLOCK_AIR);
                st.air_set++;
            }

            // flood a sub-sea-level floor back up to the waterline.
            if (kind == RAVINE_FLOOR && floor < p->sea_level) {
                int wtop = p->sea_level;
                if (wtop > CHUNK_SIZE_Y - 1) wtop = CHUNK_SIZE_Y - 1;
                for (int y = floor + 1; y <= wtop; y++) {
                    if (chunk_get_block(c, lx, y, lz) != BLOCK_AIR) continue;
                    chunk_set_block(c, lx, y, lz, BLOCK_WATER);
                    st.water_set++;
                }
            }

            // re-skin the exposed wall face. the solid cell just under the cut
            // shows the strata band for its world height — that's the visible
            // stripe on the canyon wall.
            if (kind == RAVINE_WALL) {
                int face_y = floor;
                if (face_y >= 0 && face_y < CHUNK_SIZE_Y) {
                    block_id under = chunk_get_block(c, lx, face_y, lz);
                    if (ravine_carve_is_diggable(under)) {
                        block_id band = ravine_strata_at(strata, p, face_y,
                                                         (float)wx, (float)wz);
                        if (band != under) {
                            chunk_set_block(c, lx, face_y, lz, band);
                            st.strata_faced++;
                        }
                    }
                }
            }
        }
    }
    return st;
}
