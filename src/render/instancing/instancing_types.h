#ifndef RENDER_INSTANCING_INSTANCING_TYPES_H
#define RENDER_INSTANCING_INSTANCING_TYPES_H

// shared types for the instanced draw subsystem. one place so the buffer,
// batch, cull and draw bits all agree on what a "per-instance record" is.
// the whole point of this folder: instead of one draw call per crate/torch/
// shrub, we shove a packed transform per object into a vbo and let the gpu
// stamp the same base mesh N times. classic glDrawArraysInstanced fare.

#include "../../math/vec3.h"
#include "../../math/mat4.h"
#include "../../math/aabb.h"
#include <stdint.h>

// hard ceiling for a single batch. if you blow past this we split into
// multiple draw calls (see draw.c). picked so the per-instance vbo for one
// batch stays comfortably under a meg.
#define INSTANCING_MAX_PER_BATCH   16384

// how many distinct base meshes the registry will track. props, foliage,
// debug boxes... 256 is plenty and keeps the lookup a flat array.
#define INSTANCING_MAX_MESHES      256

// sentinel mesh id. returned when a lookup misses.
#define INSTANCING_MESH_NONE       (-1)

// the per-instance gpu record. this is what actually streams into the
// instance vbo and gets read by the vertex shader via attrib divisor 1.
// we send the model matrix as four vec4 columns (mat4 is column-major here,
// so the memory layout is already what gl wants) plus a tint and a light
// scalar. 4*16 + 16 + 4 = 84 bytes; padded to 96 for alignment sanity.
typedef struct {
    float model[16];   // column-major 4x4, ready for the vertex shader
    float tint[4];     // rgba multiply, lets us recolor without new meshes
    float light;       // baked ambient term 0..1
    float _pad[3];     // keep the struct 16-byte aligned
} instancing_gpu_instance;

// the cpu-side description of one instance before we bake the matrix. we
// keep this fat and friendly; the gpu record above is the lean version we
// derive from it during the build step.
typedef struct {
    vec3   position;
    vec3   scale;
    vec3   euler;      // radians, applied x then y then z
    float  tint[4];
    float  light;
    aabb   local_box;  // model-space bounds, used for the per-instance cull
    int    mesh_id;    // which base mesh this belongs to
    uint32_t flags;
} instancing_instance;

#define INSTANCING_FLAG_ACTIVE    (1u << 0)
#define INSTANCING_FLAG_NO_CULL   (1u << 1)  // always drawn, skip frustum test
#define INSTANCING_FLAG_DIRTY     (1u << 2)  // matrix needs rebake

static inline int instancing_instance_active(const instancing_instance *it) {
    return (it->flags & INSTANCING_FLAG_ACTIVE) != 0;
}

#endif
