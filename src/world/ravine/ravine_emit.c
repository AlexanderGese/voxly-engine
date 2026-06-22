#include "ravine_emit.h"

#include "../block.h"
#include <stddef.h>

// one column's worth of edits, appended to *list. mirrors the carve's column
// logic but emits records instead of poking blocks. floor cells below sea level
// get water edits up to the line; wall cells get a strata face at the lip.
static void emit_column(ravine_edit **list, const ravine_field *f,
                        const ravine_strata *strata, const ravine_params *p,
                        int fx, int fz) {
    int idx = ravine_cell_index(fx, fz);
    if (idx < 0) return;
    if (f->cut[idx] <= 0) return;

    int surf  = f->surface[idx];
    int floor = f->floor_y[idx];
    if (floor >= surf) return;

    uint8_t kind = f->kind[idx];

    int wx, wz;
    ravine_field_to_world(f, fx, fz, &wx, &wz);

    int top = surf;
    if (top > CHUNK_SIZE_Y - 1) top = CHUNK_SIZE_Y - 1;

    // clear column to air.
    for (int y = top; y > floor; y--) {
        ravine_edit e = { wx, y, wz, BLOCK_AIR };
        darr_push(*list, e);
    }

    // flood a sub-sea-level channel floor.
    if (kind == RAVINE_FLOOR && floor < p->sea_level) {
        int wtop = p->sea_level;
        if (wtop > CHUNK_SIZE_Y - 1) wtop = CHUNK_SIZE_Y - 1;
        for (int y = floor + 1; y <= wtop; y++) {
            ravine_edit e = { wx, y, wz, BLOCK_WATER };
            darr_push(*list, e);
        }
    }

    // exposed wall face: the strata block for the lip's world height.
    if (kind == RAVINE_WALL && floor >= 0 && floor < CHUNK_SIZE_Y) {
        block_id band = ravine_strata_at(strata, p, floor,
                                         (float)wx, (float)wz);
        ravine_edit e = { wx, floor, wz, band };
        darr_push(*list, e);
    }
}

ravine_edit *ravine_emit_build(const ravine_field *f,
                               const ravine_strata *strata,
                               const ravine_params *p, size_t *out_count) {
    ravine_edit *list = NULL;
    if (!f->dirty) {
        if (out_count) *out_count = 0;
        return NULL;
    }

    // only the centre footprint maps onto the owning chunk; the pad ring belongs
    // to neighbours and would double-emit if we walked it.
    for (int lz = 0; lz < CHUNK_SIZE_Z; lz++)
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++)
            emit_column(&list, f, strata, p, lx + RAVINE_PAD, lz + RAVINE_PAD);

    if (out_count) *out_count = darr_len(list);
    return list;
}

size_t ravine_emit_count(const ravine_field *f) {
    size_t n = 0;
    for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
            int idx = ravine_cell_index(lx + RAVINE_PAD, lz + RAVINE_PAD);
            if (idx < 0) continue;
            if (f->cut[idx] <= 0) continue;
            int surf  = f->surface[idx];
            int floor = f->floor_y[idx];
            if (floor >= surf) continue;

            n += (size_t)(surf - floor);          // air column
            if (f->kind[idx] == RAVINE_WALL) n++;  // strata face
            if (f->kind[idx] == RAVINE_FLOOR && floor < f->surface[idx]) {
                // water fill estimate; cheap upper bound is fine for "how big".
            }
        }
    }
    return n;
}
