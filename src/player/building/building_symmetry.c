#include "building_symmetry.h"
#include "building_place.h"
#include "building_validate.h"
#include "building_break.h"

#include "../../config.h"

void building_symmetry_init(building_symmetry *s) {
    for (int a = 0; a < BSYM_AXIS_COUNT; a++) {
        s->enabled[a] = 0;
        s->pivot2[a] = 0;
    }
}

void building_symmetry_set(building_symmetry *s, int axis, int at) {
    if (axis < 0 || axis >= BSYM_AXIS_COUNT) return;
    s->enabled[axis] = 1;
    // plane on the low face of cell `at`: reflection maps cell c -> 2*at-1-c.
    // store 2*at so reflect() is a single subtract.
    s->pivot2[axis] = 2 * at;
}

void building_symmetry_clear(building_symmetry *s, int axis) {
    if (axis < 0 || axis >= BSYM_AXIS_COUNT) return;
    s->enabled[axis] = 0;
}

static int reflect_coord(int pivot2, int c) {
    // mirror of cell c across the plane is pivot2 - 1 - c.
    return pivot2 - 1 - c;
}

void building_symmetry_reflect(const building_symmetry *s, int axis,
                               int x, int y, int z, int *ox, int *oy, int *oz) {
    *ox = x; *oy = y; *oz = z;
    if (axis < 0 || axis >= BSYM_AXIS_COUNT || !s->enabled[axis]) return;

    switch (axis) {
        case BSYM_AXIS_X: *ox = reflect_coord(s->pivot2[axis], x); break;
        case BSYM_AXIS_Z: *oz = reflect_coord(s->pivot2[axis], z); break;
        case BSYM_AXIS_Y: *oy = reflect_coord(s->pivot2[axis], y); break;
        default: break;
    }
}

int building_symmetry_expand(const building_symmetry *s, int x, int y, int z,
                             int out[8][3]) {
    // start with the source cell, then for each enabled axis double the set by
    // appending the reflection of every cell collected so far. classic powerset
    // build; dedup at the end since coincident planes can collapse images.
    int set[8][3];
    int n = 0;
    set[n][0] = x; set[n][1] = y; set[n][2] = z; n++;

    for (int axis = 0; axis < BSYM_AXIS_COUNT; axis++) {
        if (!s->enabled[axis]) continue;
        int base = n;
        for (int i = 0; i < base && n < 8; i++) {
            int rx, ry, rz;
            building_symmetry_reflect(s, axis, set[i][0], set[i][1], set[i][2],
                                      &rx, &ry, &rz);
            set[n][0] = rx; set[n][1] = ry; set[n][2] = rz;
            n++;
        }
    }

    // dedup into out[].
    int m = 0;
    for (int i = 0; i < n; i++) {
        int dup = 0;
        for (int j = 0; j < m; j++) {
            if (out[j][0] == set[i][0] && out[j][1] == set[i][1] &&
                out[j][2] == set[i][2]) { dup = 1; break; }
        }
        if (dup) continue;
        out[m][0] = set[i][0];
        out[m][1] = set[i][1];
        out[m][2] = set[i][2];
        m++;
    }
    return m;
}

int building_symmetry_place(world *w, building_history *hist,
                            const building_symmetry *s, block_id id,
                            int x, int y, int z, int face, vec3 feet) {
    int cells[8][3];
    int n = building_symmetry_expand(s, x, y, z, cells);
    int placed = 0;

    for (int i = 0; i < n; i++) {
        int cx = cells[i][0], cy = cells[i][1], cz = cells[i][2];

        // reflecting the face keeps oriented blocks looking right across an
        // x or z mirror. y mirror leaves horizontal facing alone.
        int f = face;
        if (s->enabled[BSYM_AXIS_X] && (cx != x))
            f = (f == BFACE_PX) ? BFACE_NX : (f == BFACE_NX) ? BFACE_PX : f;
        if (s->enabled[BSYM_AXIS_Z] && (cz != z))
            f = (f == BFACE_PZ) ? BFACE_NZ : (f == BFACE_NZ) ? BFACE_PZ : f;

        if (building_validate_place(w, id, cx, cy, cz, f, feet) != BPLACE_OK)
            continue;

        block_id before = world_get_block(w, cx, cy, cz);
        world_set_block(w, cx, cy, cz, id);
        building_mark_dirty(w, cx, cy, cz);

        building_edit e = { cx, cy, cz, before, id };
        if (hist) building_history_record(hist, &e);
        placed++;
    }
    return placed;
}

int building_symmetry_break(world *w, building_history *hist,
                            const building_symmetry *s, int x, int y, int z) {
    int cells[8][3];
    int n = building_symmetry_expand(s, x, y, z, cells);
    int broke = 0;

    for (int i = 0; i < n; i++) {
        int cx = cells[i][0], cy = cells[i][1], cz = cells[i][2];
        if (cy < 0 || cy >= CHUNK_SIZE_Y) continue;

        block_id id = world_get_block(w, cx, cy, cz);
        if (!building_is_breakable(id)) continue;

        world_set_block(w, cx, cy, cz, BLOCK_AIR);
        building_mark_dirty(w, cx, cy, cz);

        building_edit e = { cx, cy, cz, id, BLOCK_AIR };
        if (hist) building_history_record(hist, &e);
        broke++;
    }
    return broke;
}
