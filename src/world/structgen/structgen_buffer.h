#ifndef WORLD_STRUCTGEN_BUFFER_H
#define WORLD_STRUCTGEN_BUFFER_H

#include "structgen_types.h"

// growable voxel buffer for structure output. unlike gen2_place_buf (which is
// caller-owned, fixed) this one owns its storage and grows, because a village
// can emit a few thousand voxels and we dont want the caller to guess the cap.
// uses the darray growth pattern but keeps an explicit struct so the api reads
// cleaner at the call sites in the piece builders.

typedef struct {
    structgen_voxel *items;
    int count;
    int cap;
} structgen_buffer;

void structgen_buffer_init(structgen_buffer *b);
void structgen_buffer_free(structgen_buffer *b);
void structgen_buffer_reset(structgen_buffer *b);   // keep storage

// add one voxel. grows as needed. returns 1, or 0 on alloc failure.
int  structgen_buffer_add(structgen_buffer *b, int x, int y, int z, block_id id);

// fill a solid box [x0,x1) x [y0,y1) x [z0,z1) with id. returns count added.
int  structgen_buffer_fill_box(structgen_buffer *b, structgen_box box, block_id id);

// fill only the hollow shell (walls/floor/ceiling) of a box. interior left
// untouched. returns count added.
int  structgen_buffer_fill_shell(structgen_buffer *b, structgen_box box, block_id id);

// outline a single y-layer rectangle (perimeter only).
int  structgen_buffer_fill_frame(structgen_buffer *b, structgen_box box, int y, block_id id);

#endif
