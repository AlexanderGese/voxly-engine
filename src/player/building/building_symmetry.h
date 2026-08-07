#ifndef PLAYER_BUILDING_SYMMETRY_H
#define PLAYER_BUILDING_SYMMETRY_H
#include "building_types.h"
#include "building_history.h"
#include "../../math/vec3.h"
#include "../../world/world.h"
enum {
    BSYM_AXIS_X = 0,   // mirror across a plane of constant x
    BSYM_AXIS_Z,       // ...constant z
    BSYM_AXIS_Y,       // ...constant y (rare, but free to support)
    BSYM_AXIS_COUNT
}
;
typedef struct {
    int enabled[BSYM_AXIS_COUNT];
    // doubled pivot per axis: reflection of cell c is (pivot2 - 1 - c). storing
    // 2*p lets the plane sit either on a block face (even) or mid-block (odd).
    int pivot2[BSYM_AXIS_COUNT];
} building_symmetry;
void building_symmetry_init(building_symmetry *s);
void building_symmetry_set(building_symmetry *s, int axis, int at);
void building_symmetry_clear(building_symmetry *s, int axis);
void building_symmetry_reflect(const building_symmetry *s, int axis,
                               int x, int y, int z, int *ox, int *oy, int *oz);
int building_symmetry_expand(const building_symmetry *s, int x, int y, int z,
                             int out[8][3]);
int building_symmetry_place(world *w, building_history *hist,
                            const building_symmetry *s, block_id id,
                            int x, int y, int z, int face, vec3 feet);
int building_symmetry_break(world *w, building_history *hist,
                            const building_symmetry *s, int x, int y, int z);
#endif
