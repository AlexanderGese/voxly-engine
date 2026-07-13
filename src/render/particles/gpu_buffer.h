#ifndef RENDER_PARTICLES_GPU_BUFFER_H
#define RENDER_PARTICLES_GPU_BUFFER_H

// the gpu side: one vao/vbo holding the expanded billboard verts. we use a
// single dynamic vbo and orphan it each frame (glBufferData with NULL then
// re-upload) which is the boring-but-portable streaming pattern. the engine
// already does GL_STATIC_DRAW for chunks; particles want GL_STREAM_DRAW.

#include "../gl.h"
#include "particle_types.h"

typedef struct {
    glid vao, vbo;
    int  capacity_verts;   // how many verts the vbo can currently hold
    int  vert_count;       // verts uploaded this frame
    int  initialised;
} particles_gpu_buffer;

void particles_gpu_init(particles_gpu_buffer *g, int initial_verts);
void particles_gpu_destroy(particles_gpu_buffer *g);

// stream `count` verts into the vbo, growing it if needed. safe to call with
// count==0 (just resets vert_count).
void particles_gpu_upload(particles_gpu_buffer *g,
                          const particles_vertex *verts, int count);

// issue the draw. assumes the caller has bound the right shader/texture and
// set blend state. draws GL_TRIANGLES over the whole uploaded buffer.
void particles_gpu_draw(const particles_gpu_buffer *g);

#endif
