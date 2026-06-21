#ifndef WORLD_MINESHAFT_BUFFER_H
#define WORLD_MINESHAFT_BUFFER_H

#include "mineshaft_types.h"
#include "mineshaft_box.h"

// growable voxel buffer owning its storage. the builders emit thousands of
// voxels for a sprawling shaft and the caller shouldn't have to guess a cap.
// same doubling-realloc pattern the rest of the worldgen uses, wrapped in an
// explicit struct so the call sites in the builders read cleanly.

typedef struct {
    mineshaft_voxel *items;
    int count;
    int cap;
} mineshaft_buffer;

void mineshaft_buffer_init(mineshaft_buffer *b);
void mineshaft_buffer_free(mineshaft_buffer *b);
void mineshaft_buffer_reset(mineshaft_buffer *b);   // keep storage, drop count

// add one voxel. grows as needed. returns 1, or 0 on alloc failure.
int  mineshaft_buffer_add(mineshaft_buffer *b, int x, int y, int z, block_id id);

// fill a solid box [x0,x1) etc with id. returns count added.
int  mineshaft_buffer_fill_box(mineshaft_buffer *b, mineshaft_box box, block_id id);

// hollow out a box to air but lay a floor/ceiling/walls shell of id. interior
// becomes air. this is the bread-and-butter "dig a room" primitive. returns
// total voxels written (shell + air).
int  mineshaft_buffer_carve_room(mineshaft_buffer *b, mineshaft_box box,
                                 block_id wall, block_id air);

// stamp a single y-layer rectangle outline (perimeter only).
int  mineshaft_buffer_frame(mineshaft_buffer *b, mineshaft_box box, int y, block_id id);

// vertical post from y0..y1 (exclusive) at (x,z). returns count.
int  mineshaft_buffer_post(mineshaft_buffer *b, int x, int y0, int y1, int z, block_id id);

// horizontal line along an axis. axis 0=x, 2=z. returns count.
int  mineshaft_buffer_line(mineshaft_buffer *b, int x, int y, int z,
                           int axis, int len, block_id id);

#endif
