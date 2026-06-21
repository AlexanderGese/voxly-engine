#ifndef WORLD_MINESHAFT_GRID_H
#define WORLD_MINESHAFT_GRID_H
#include <stdint.h>
#include "mineshaft_types.h"
// the abstract maze: a 2d grid of cells, each tagged with a kind and a bitmask
// of which neighbours it links to. the carver writes into this; the builder
// reads it cell by cell and turns each into voxels. keeping the abstract layout
// separate from the voxel stamping is the whole trick - we can reason about
// connectivity, junctions and dead ends without ever touching a block.
#define MINESHAFT_GRID_MAX  16   // cap on grid dims, keeps cells[] static
// per-cell kind. distinct from the public mineshaft_piece_type (which is the
// coarse "what builder runs" tag); this is the finer maze-topology classification
// the build pass dispatches on. junction/deadend fall out of link degree.
typedef enum {
    MS_CELL_EMPTY = 0,   // never visited, solid rock stays
    MS_CELL_CORRIDOR,    // a plain dug passage
    MS_CELL_JUNCTION,    // 3+ exits, gets a crossing beam
    MS_CELL_ROOM,        // widened ore room, exposed ore + a chest
    MS_CELL_SHAFT,       // vertical drop with a ladder, hooks a lower level
    MS_CELL_DEADEND      // a stub the walk abandoned, cave-in candidate
} mineshaft_cell_kind;
// bit per direction, OR'd into a cell to record neighbour links. the maze writes
// these as it carves; the builder reads them to knock holes in the right walls.
enum {
    MS_LINK_N = 1 << MINESHAFT_NORTH,
    MS_LINK_E = 1 << MINESHAFT_EAST,
    MS_LINK_S = 1 << MINESHAFT_SOUTH,
    MS_LINK_W = 1 << MINESHAFT_WEST
}
;
// per-cell flags set by the classify / build passes.
enum {
    MS_FLAG_LADDER    = 1 << 0,   // hosts a vertical ladder
    MS_FLAG_COLLAPSED = 1 << 1,   // partial cave-in
    MS_FLAG_CHEST     = 1 << 2    // room got a loot chest marker
}
;
typedef struct {
    uint8_t kind;     // mineshaft_cell_kind
    uint8_t links;    // OR of MS_LINK_* bits
    uint8_t depth;    // bfs distance from the entry cell
    uint8_t flags;    // OR of MS_FLAG_* bits
} mineshaft_cell;
typedef struct {
    int w, d;                                  // active dims (<= GRID_MAX)
    int entry_x, entry_z;                      // cell the maze grew from
    mineshaft_cell cells[MINESHAFT_GRID_MAX * MINESHAFT_GRID_MAX];
} mineshaft_grid;
void mineshaft_grid_init(mineshaft_grid *g, int w, int d);
// bounds-checked accessor. out-of-range get returns a shared zeroed cell so
// callers can read .kind/.links without branching everywhere (must not write it).
mineshaft_cell *mineshaft_grid_at(mineshaft_grid *g, int x, int z);
int  mineshaft_grid_in_bounds(const mineshaft_grid *g, int x, int z);
// count how many of the four cardinal neighbours this cell links to.
int  mineshaft_grid_degree(const mineshaft_grid *g, int x, int z);
// link two adjacent cells both ways. no-op if not adjacent / out of bounds.
void mineshaft_grid_link(mineshaft_grid *g, int x, int z, mineshaft_dir d);
#endif
