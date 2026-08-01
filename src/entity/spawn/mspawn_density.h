#ifndef ENTITY_SPAWN_MSPAWN_DENSITY_H
#define ENTITY_SPAWN_MSPAWN_DENSITY_H

#include "mspawn_types.h"
#include "../mob.h"

// density bookkeeping. two jobs: global per-category caps (so the world cant
// fill with zombies) and a coarse spatial grid (so a single tick cant dump a
// hundred mobs into one chunk while the rest of the loaded area sits empty).
//
// the grid is a hash of cells around the player; we only ever care about local
// crowding so theres no need to cover the whole world. cells are MSPAWN_CELL
// blocks on a side.

#define MSPAWN_CELL        16   // one chunk footprint per cell
#define MSPAWN_GRID_DIM    24   // cells per side of the tracked window
#define MSPAWN_GRID_CELLS  (MSPAWN_GRID_DIM * MSPAWN_GRID_DIM)

// soft ceilings per category. these are the population the driver tries to hold
// the loaded area at, not hard limits on the registry.
#define MSPAWN_CAP_PASSIVE 16
#define MSPAWN_CAP_HOSTILE 32
#define MSPAWN_CAP_AMBIENT  8

// how many mobs of the same category one cell tolerates before we refuse to
// add more there. keeps packs from stacking on a single tile.
#define MSPAWN_CELL_MAX     4

typedef struct {
    int   cat_count[MSPAWN_CAT_COUNT];      // live tally per category
    int   cat_cap[MSPAWN_CAT_COUNT];        // resolved ceilings
    // per-cell occupancy, indexed [cat][cell]. small ints, rebuilt each pass.
    uint8_t cell[MSPAWN_CAT_COUNT][MSPAWN_GRID_CELLS];
    int   origin_cx, origin_cz;             // grid origin in cell coords
} mspawn_density;

// reset caps to defaults. call once at init.
void mspawn_density_init(mspawn_density *d);

// recount everything from the live registry, centering the grid on the player.
// cheap enough to do every spawn pass (MAX_MOBS is tiny).
void mspawn_density_rebuild(mspawn_density *d, const mob_registry *mr,
                            vec3 player_pos);

// is there room, globally, for one more of this category?
int  mspawn_density_has_room(const mspawn_density *d, mspawn_category cat);

// is the cell containing this world position below its per-cell ceiling for
// the category? positions outside the tracked window are treated as full so we
// never spawn where we cant see crowding.
int  mspawn_density_cell_ok(const mspawn_density *d, mspawn_category cat,
                            vec3 pos);

// fold a freshly-committed spawn back into the tallies so a single pass that
// adds many mobs still respects the caps mid-loop.
void mspawn_density_account(mspawn_density *d, mspawn_category cat, vec3 pos);

#endif
