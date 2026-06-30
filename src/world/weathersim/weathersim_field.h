#ifndef WORLD_WEATHERSIM_FIELD_H
#define WORLD_WEATHERSIM_FIELD_H

#include <stddef.h>
#include "weathersim_types.h"

// the coarse grid the simulator scribbles on. a flat WEATHERSIM_DIM^2 array of
// cells plus a scratch copy for the double-buffered diffusion sweep. the field
// is anchored at a cell-space origin (the lowest-left cell's world coords) and
// slides as the player roams, scrolling rows/cols in and seeding the freshly
// exposed strip from climate.
//
// keeping it array-of-structs (not parallel planes like rivers) was a coin
// flip; the diffusion pass touches every member of a cell at once so locality
// wins here, whereas rivers sweeps one plane at a time. horses, courses.

typedef struct {
    weathersim_cell cells[WEATHERSIM_CELLS];
    weathersim_cell scratch[WEATHERSIM_CELLS]; // ping-buffer for diffusion

    int   origin_cx;   // cell-space x of cells[0]
    int   origin_cz;   // cell-space z of cells[0]
    int   seeded;      // 0 until first climate fill, then 1
} weathersim_field;

// flat index + bounds in grid coords (0..DIM-1).
static inline int weathersim_field_idx(int gx, int gz) {
    return gz * WEATHERSIM_DIM + gx;
}
int weathersim_field_in_bounds(int gx, int gz);

// world<->cell<->grid coordinate plumbing. cell coords are global (independent
// of the field window); grid coords are the local 0..DIM-1 slot.
int  weathersim_world_to_cell(int wx);
void weathersim_cell_center_world(int cell_x, int cell_z, int *wx, int *wz);

// grab a cell by grid coords. out-of-bounds clamps to the edge so the
// diffusion stencil never reads garbage.
weathersim_cell *weathersim_field_at(weathersim_field *f, int gx, int gz);
const weathersim_cell *weathersim_field_at_const(const weathersim_field *f,
                                                 int gx, int gz);

// translate a global cell coord into a grid slot. returns 0 if that cell isn't
// inside the current window.
int weathersim_field_cell_to_grid(const weathersim_field *f,
                                  int cell_x, int cell_z, int *gx, int *gz);

// zero everything and park the window so its center cell sits on center_cell.
void weathersim_field_init(weathersim_field *f, int center_cx, int center_cz);

// shift the window so center_cell becomes the middle cell again. scrolls the
// existing rows/cols that still overlap and marks the freshly exposed strip
// with a sentinel temp (NAN) so the climate pass knows to re-seed only those.
// returns the number of cells that need re-seeding.
int weathersim_field_recenter(weathersim_field *f, int center_cx, int center_cz);

#endif
