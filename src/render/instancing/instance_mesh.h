#ifndef RENDER_INSTANCING_INSTANCE_MESH_H
#define RENDER_INSTANCING_INSTANCE_MESH_H

// a base mesh set up for instanced drawing. owns the vao that ties the static
// geometry vbo (loc 0..2: pos/uv/light, same layout as render/mesh.h) to a
// per-instance buffer (loc 3..8: model columns, tint, light). build once,
// draw many.

#include "../gl.h"
#include "instancing_types.h"
#include "instance_buffer.h"

// the static base vertex. matches the engine's `vertex` from render/mesh.h so
// existing meshes can be handed straight in. duplicated here rather than
// including mesh.h so this folder stays self-contained.
typedef struct {
    float x, y, z;
    float u, v;
    float light;
} instancing_base_vertex;

typedef struct {
    glid vao;
    glid base_vbo;     // static geometry, owned here
    int  base_count;   // vertices in the base mesh
    aabb local_box;    // model-space bounds of the base geometry

    instancing_instance_buffer inst;  // per-instance stream buffer
    int  uploaded;     // base geometry has been uploaded at least once
} instancing_mesh;

// create the vao + buffers and wire all attributes. base geometry is uploaded
// from `verts`. `box` is the model-space aabb (used as the default per-
// instance cull box). pass initial_instances to presize the stream buffer.
void instancing_mesh_init(instancing_mesh *m,
                          const instancing_base_vertex *verts, int count,
                          aabb box, int initial_instances);

void instancing_mesh_destroy(instancing_mesh *m);

// push this frame's baked instance records into the per-instance vbo.
void instancing_mesh_set_instances(instancing_mesh *m,
                                   const instancing_gpu_instance *recs,
                                   int count);

// bind the vao. used by the draw layer before issuing the instanced call.
void instancing_mesh_bind(const instancing_mesh *m);

int  instancing_mesh_instance_count(const instancing_mesh *m);

#endif
