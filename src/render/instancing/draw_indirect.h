#ifndef RENDER_INSTANCING_DRAW_INDIRECT_H
#define RENDER_INSTANCING_DRAW_INDIRECT_H

// the indirect submission path. instead of one glDrawArraysInstanced per mesh
// group (draw.c), we pack every group's parameters into a command buffer and
// fire the whole batch with a single glMultiDrawArraysIndirect. fewer driver
// round-trips when you have lots of small groups (a forest of 30 prop types).
//
// the catch: MDI reads vertex/instance offsets from one big vbo, so this path
// only works when all the base meshes share a vertex buffer and all instance
// records share an instance buffer. we maintain exactly that here — a combined
// vao plus a merged instance vbo — and the registry meshes are referenced only
// for their base vertex ranges. if you don't need this, just use draw.c.
//
// command layout matches GL's DrawArraysIndirectCommand exactly so we can
// memcpy our array straight into the GL_DRAW_INDIRECT_BUFFER.

#include "../gl.h"
#include "batch.h"
#include "registry.h"
#include "instancing_stats.h"

// mirrors the gl struct: count, instanceCount, first, baseInstance. order and
// width are load-bearing — gl reads it raw.
typedef struct {
    GLuint count;          // verts in the base mesh
    GLuint instance_count; // how many instances of it
    GLuint first;          // first vertex in the shared base vbo
    GLuint base_instance;  // first instance record in the merged instance vbo
} instancing_draw_cmd;

typedef struct {
    glid vao;            // shared: base attribs + per-instance attribs
    glid base_vbo;       // all base meshes concatenated
    glid inst_vbo;       // all instance records concatenated
    glid cmd_vbo;        // GL_DRAW_INDIRECT_BUFFER

    // cpu mirrors. base vertices are uploaded once when meshes change; the
    // instance + command arrays are restreamed each frame.
    int  base_capacity;  // verts the base vbo can hold
    int  inst_capacity;  // records the instance vbo can hold
    int  cmd_capacity;   // commands the cmd vbo can hold

    // where each registry mesh id lives in the shared base vbo: first vertex
    // and vertex count. -1 first means "not packed yet".
    int  mesh_first[INSTANCING_MAX_MESHES];
    int  mesh_count[INSTANCING_MAX_MESHES];

    int  initialised;
    int  ncommands;      // commands submitted last frame
} instancing_indirect;

void instancing_indirect_init(instancing_indirect *ind);
void instancing_indirect_destroy(instancing_indirect *ind);

// (re)pack the registry's base meshes into the shared base vbo and record each
// mesh's vertex range. call whenever the set of registered meshes changes —
// not every frame.
void instancing_indirect_pack_bases(instancing_indirect *ind,
                                    instancing_registry *reg);

// build the command buffer + merged instance buffer from a built batch and
// draw it all with one multi-draw call. base vertex ranges come from the last
// pack. updates stats.
void instancing_indirect_submit(instancing_indirect *ind,
                                const instancing_batch *batch,
                                instancing_stats *stats);

#endif
