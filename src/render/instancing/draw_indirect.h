#ifndef RENDER_INSTANCING_DRAW_INDIRECT_H
#define RENDER_INSTANCING_DRAW_INDIRECT_H
#include "../gl.h"
#include "batch.h"
#include "registry.h"
#include "instancing_stats.h"
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
#endif
