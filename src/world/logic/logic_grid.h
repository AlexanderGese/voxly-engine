#ifndef WORLD_LOGIC_GRID_H
#define WORLD_LOGIC_GRID_H

#include "logic_types.h"
#include "../../util/hashmap.h"

// the sparse store of logic cells. only blocks that actually carry logic get a
// cell, keyed by logic_key(x,y,z). the cells themselves live in a pool we own
// (a plain growable array of blocks) and the hashmap maps key -> cell*.
//
// we never hand out cell pointers across an insert that could grow the pool,
// because we hand out *stable* pointers: cells live in fixed-size blocks that
// are never realloc'd once allocated. a freelist recycles removed slots.

#define LOGIC_GRID_BLOCK 256   // cells per pool block

typedef struct logic_grid_block {
    logic_cell cells[LOGIC_GRID_BLOCK];
    struct logic_grid_block *next;
} logic_grid_block;

typedef struct {
    hashmap          map;        // key -> logic_cell*
    logic_grid_block *blocks;    // pool, singly linked
    int              block_used; // cells used in the head block
    logic_cell      **freelist;  // recycled cell slots (darray)
    int              count;      // live cells
} logic_grid;

void logic_grid_init(logic_grid *g);
void logic_grid_free(logic_grid *g);

// fetch the cell at a coord, or NULL. cheap, no allocation.
logic_cell *logic_grid_get(logic_grid *g, int x, int y, int z);

// insert (or fetch existing) a cell of a given kind. returns a stable pointer.
// if a cell already exists at the coord it is returned as-is regardless of
// kind, so callers should check ->kind if they care.
logic_cell *logic_grid_put(logic_grid *g, int x, int y, int z, block_id kind);

// remove the cell at a coord if present. returns 1 if something was removed.
int logic_grid_remove(logic_grid *g, int x, int y, int z);

int logic_grid_count(const logic_grid *g);

// iterate every live cell. the callback must not insert/remove during the walk
// (it may mutate fields in place). user is passed through.
typedef void (*logic_grid_visit_fn)(logic_cell *c, void *user);
void logic_grid_each(logic_grid *g, logic_grid_visit_fn fn, void *user);

#endif
