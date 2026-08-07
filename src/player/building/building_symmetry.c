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
    // build;
dedup at the end since coincident planes can collapse images.
    int set[8][3];
int n = 0;
set[n][0] = x;
set[n][1] = y;
set[n][2] = z;
n++;
for (int axis = 0;
axis < BSYM_AXIS_COUNT;
axis++) {
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
for (int i = 0;
i < n;
int n = building_symmetry_expand(s, x, y, z, cells);
int broke = 0;
for (int i = 0;
i < n;
}
