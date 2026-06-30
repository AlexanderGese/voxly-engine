#ifndef WORLD_WEATHERSIM_FIELD_H
#define WORLD_WEATHERSIM_FIELD_H
#include <stddef.h>
#include "weathersim_types.h"
typedef struct {
    weathersim_cell cells[WEATHERSIM_CELLS];
    weathersim_cell scratch[WEATHERSIM_CELLS]; // ping-buffer for diffusion

    int   origin_cx;   // cell-space x of cells[0]
    int   origin_cz;   // cell-space z of cells[0]
    int   seeded;      // 0 until first climate fill, then 1
} weathersim_field;
#endif
