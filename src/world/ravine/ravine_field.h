#ifndef WORLD_RAVINE_FIELD_H
#define WORLD_RAVINE_FIELD_H
#include <stdint.h>
#include "ravine_types.h"
typedef struct {
    int base_x;     // world x of cell (RAVINE_PAD, RAVINE_PAD)
    int base_z;
    int chunk_cx;
    int chunk_cz;
} ravine_origin;
#endif
