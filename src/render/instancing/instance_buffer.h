#ifndef RENDER_INSTANCING_INSTANCE_BUFFER_H
#define RENDER_INSTANCING_INSTANCE_BUFFER_H

// the gpu side of an instance set: one dynamic vbo holding the packed
// instancing_gpu_instance records. we orphan-and-restream every frame, same
// streaming pattern the particle gpu_buffer uses. this buffer does NOT own a
// vao — the vao lives on the instance_mesh, which wires both the static base
// mesh attribs and this buffer's per-instance attribs together.

#include "../gl.h"
#include "instancing_types.h"

typedef struct {
    glid vbo;
    int  capacity;     // records the vbo can currently hold
    int  count;        // records uploaded this frame
    int  initialised;
} instancing_instance_buffer;

void instancing_buffer_init(instancing_instance_buffer *b, int initial);
void instancing_buffer_destroy(instancing_instance_buffer *b);

// stream `count` records into the vbo, growing (with 2x headroom) if needed.
// orphans the previous storage so we don't stall on last frame's draw. safe
// with count==0 (just zeroes count). assumes the records are already baked.
void instancing_buffer_upload(instancing_instance_buffer *b,
                              const instancing_gpu_instance *recs, int count);

// wire the per-instance vertex attributes onto the currently bound vao.
// caller must have a vao bound and this buffer's vbo bound to GL_ARRAY_BUFFER.
// uses attrib locations base..base+4 with divisor 1. returns next free loc.
int  instancing_buffer_setup_attribs(int base_location);

#endif
