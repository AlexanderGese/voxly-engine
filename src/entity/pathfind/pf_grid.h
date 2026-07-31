#ifndef ENTITY_PATHFIND_PF_GRID_H
#define ENTITY_PATHFIND_PF_GRID_H

#include "pf_types.h"
#include "../../world/world.h"

// a walkability view of the world clipped to a window around the start.
// we don't copy blocks; we just answer "can a mob stand here" and "what y
// would it stand at" by poking the world directly, with a small cache so
// the heap doesn't re-query the same column a hundred times.

// per-column cache slot. resolved lazily.
typedef struct {
    int8_t  resolved;     // 0 = not looked at yet
    int8_t  standable;    // 1 = there's a floor a mob can stand on
    int16_t floor_y;      // y of the block the feet rest on (if standable)
} pf_column;

typedef struct {
    world     *w;
    pf_coord   origin;    // min corner of the window in block space
    int        h;         // window height we scan when resolving columns
    pf_column  cols[PF_WINDOW_SIZE * PF_WINDOW_SIZE];
} pf_grid;

// set up a grid window centered on `center`. origin is derived so center
// sits in the middle. clears the column cache.
void pf_grid_init(pf_grid *g, world *w, pf_coord center);

// is (lx,lz) inside the window? local coords, 0..PF_WINDOW_SIZE-1.
int  pf_grid_in_bounds(const pf_grid *g, int lx, int lz);

// convert world block coord -> local window coord. returns 0 if outside.
int  pf_grid_to_local(const pf_grid *g, pf_coord wc, int *lx, int *lz);

// can a mob stand in this column near height `near_y`? if so, *floor_out
// gets the y of the floor block it rests on. uses/fills the column cache.
int  pf_grid_standable(pf_grid *g, int lx, int lz, int near_y, int *floor_out);

// raw walkability test at an exact block: air at feet+head, solid below.
int  pf_grid_walkable_at(world *w, int wx, int wy, int wz);

#endif
