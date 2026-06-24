#ifndef WORLD_RIVERS_FIELD_H
#define WORLD_RIVERS_FIELD_H
#include <stdint.h>
#include "rivers_types.h"
typedef struct {
    int   base_x;     // world x of cell (RIVERS_PAD, RIVERS_PAD)
    int   base_z;
    int   chunk_cx;
    int   chunk_cz;
} rivers_origin;
typedef struct {
    rivers_origin origin;

    int     surface[RIVERS_CELLS];   // input land height
    int     filled [RIVERS_CELLS];   // post depression-fill height
    float   accum  [RIVERS_CELLS];   // flow accumulation
    uint8_t dir    [RIVERS_CELLS];   // rivers_dir
    uint8_t wet    [RIVERS_CELLS];   // rivers_wet
    int     water_y[RIVERS_CELLS];   // water surface y where wet

    int     dirty;                   // got any wet cells at all
} rivers_field;
#endif
