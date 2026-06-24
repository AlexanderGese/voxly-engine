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
rivers_field *rivers_field_create(void);
void          rivers_field_destroy(rivers_field *f);
void rivers_field_reset(rivers_field *f, rivers_origin origin);
int  rivers_field_idx(int x, int z);
int  rivers_field_in_bounds(int x, int z);
void rivers_field_set_surface(rivers_field *f, int x, int z, int y);
int  rivers_field_surface(const rivers_field *f, int x, int z);
int  rivers_field_on_edge(int x, int z);
void rivers_field_to_world(const rivers_field *f, int x, int z,
                           int *wx, int *wz);
int  rivers_field_from_world(const rivers_field *f, int wx, int wz,
                             int *x, int *z);
int  rivers_field_count_wet(const rivers_field *f, uint8_t state);
#endif
