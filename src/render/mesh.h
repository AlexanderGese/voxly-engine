#ifndef RENDER_MESH_H
#define RENDER_MESH_H

#include "gl.h"
#include <stdint.h>

// packed vertex. 5*4 bytes each.
// x, y, z  (float)
// u, v     (float)
// tile     (packed uint32: tile_id | light<<16)
// — actually no i was overthinking, here's what we use:
// x, y, z  float
// u, v     float
// light    float (0..1)

typedef struct {
    float x, y, z;
    float u, v;
    float light;
} vertex;

typedef struct {
    glid      vao, vbo;
    vertex   *verts;   // dynamic array
    int       count;
    int       cap;
    int       uploaded;
} mesh;

void mesh_init(mesh *m);
void mesh_reset(mesh *m);
void mesh_push_vertex(mesh *m, vertex v);
void mesh_upload(mesh *m);
void mesh_draw(const mesh *m);
void mesh_destroy(mesh *m);

#endif
