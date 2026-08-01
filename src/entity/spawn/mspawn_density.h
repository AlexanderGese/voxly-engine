#ifndef ENTITY_SPAWN_MSPAWN_DENSITY_H
#define ENTITY_SPAWN_MSPAWN_DENSITY_H
#include "mspawn_types.h"
#include "../mob.h"
#define MSPAWN_CELL        16   // one chunk footprint per cell
#define MSPAWN_GRID_DIM    24   // cells per side of the tracked window
#define MSPAWN_GRID_CELLS  (MSPAWN_GRID_DIM * MSPAWN_GRID_DIM)
#define MSPAWN_CAP_PASSIVE 16
#define MSPAWN_CAP_HOSTILE 32
#define MSPAWN_CAP_AMBIENT  8
#define MSPAWN_CELL_MAX     4
typedef struct {
    int   cat_count[MSPAWN_CAT_COUNT];      // live tally per category
    int   cat_cap[MSPAWN_CAT_COUNT];        // resolved ceilings
    // per-cell occupancy, indexed [cat][cell]. small ints, rebuilt each pass.
    uint8_t cell[MSPAWN_CAT_COUNT][MSPAWN_GRID_CELLS];
    int   origin_cx, origin_cz;             // grid origin in cell coords
} mspawn_density;
void mspawn_density_init(mspawn_density *d);
void mspawn_density_rebuild(mspawn_density *d, const mob_registry *mr,
                            vec3 player_pos);
#endif
