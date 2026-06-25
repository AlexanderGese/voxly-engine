#ifndef WORLD_STRONGHOLD_BUFFER_H
#define WORLD_STRONGHOLD_BUFFER_H

#include "stronghold_types.h"
#include "stronghold_box.h"

// growable voxel buffer for stronghold output. owns its storage and grows; a
// full stronghold emits tens of thousands of voxels so we never want the caller
// to guess a cap. same darray growth pattern as structgen_buffer, kept as an
// explicit struct so the carver call sites read cleanly.

typedef struct {
    stronghold_voxel *items;
    int count;
    int cap;
} stronghold_buffer;

void stronghold_buffer_init(stronghold_buffer *b);
void stronghold_buffer_free(stronghold_buffer *b);
void stronghold_buffer_reset(stronghold_buffer *b);   // keep storage

// add one voxel. grows as needed. returns 1, or 0 on alloc failure.
int  stronghold_buffer_add(stronghold_buffer *b, int x, int y, int z, block_id id);

// solid box [x0,x1) x [y0,y1) x [z0,z1). returns count added.
int  stronghold_buffer_fill_box(stronghold_buffer *b, stronghold_box box, block_id id);

// hollow shell only (walls/floor/ceiling), interior untouched.
int  stronghold_buffer_fill_shell(stronghold_buffer *b, stronghold_box box, block_id id);

// carve: overwrite a box with air. (just fill_box with BLOCK_AIR, named for
// intent at the call sites where we hollow a room out of solid stone.)
int  stronghold_buffer_carve(stronghold_buffer *b, stronghold_box box);

// outline one y-layer rectangle (perimeter only). floors-with-borders, etc.
int  stronghold_buffer_fill_frame(stronghold_buffer *b, stronghold_box box, int y, block_id id);

// vertical post from (x,y0,z) up to (but not including) y1.
int  stronghold_buffer_fill_column(stronghold_buffer *b, int x, int y0, int y1, int z, block_id id);

#endif
