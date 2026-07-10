#ifndef RENDER_INSTANCING_TYPES_H
#define RENDER_INSTANCING_TYPES_H
#include "../../math/vec3.h"
#include "../../math/mat4.h"
#include "../../math/aabb.h"
#include "../gl.h"
#include <stdint.h>
// shared types for the instanced-draw subsystem. one base mesh, many
// copies of it scattered around the world (think: foliage, rocks, mob
// props, particles-that-are-actually-geometry). instead of N draw calls
// we stuff a per-instance attribute buffer and let the gpu replicate.
//
// nothing in this header touches GL state, it just describes the data.
// the heavy lifting lives in instance_*.c. kept the structs plain so the
// frustum/sort/draw bits all agree on the layout without a web of getters.
// hard ceiling on instances we'll feed to a single draw call. above this
// we split into multiple draws. picked so the streaming vbo stays sane on
// older drivers — 64k * 80 bytes is ~5MB, which is plenty.
#define INSTANCING_MAX_PER_DRAW   65536
// how many base meshes the manager can track at once. it's a flat array,
// not a hashmap, because in practice we have a dozen prop types tops.
#define INSTANCING_MAX_MESHES     64
// per-instance payload as it lands in the gpu buffer. this is the exact
// byte layout uploaded to the instance vbo, so DO NOT reorder without
// fixing instance_mesh.c's attrib pointers. 80 bytes, 16-float aligned-ish.
// model   : 4x4 column-major transform           (64 bytes)
// tint    : rgb multiply, a unused-for-now        (16 bytes)
typedef struct {
    float model[16];
    float tint[4];
} instance_gpu;
typedef struct {
    vec3  pos;        // world position of the instance origin
    vec3  scale;      // per-axis scale
    float yaw;        // rotation about +y, radians. most props only need this
    vec3  tint;       // colour multiply
    float radius;     // bounding sphere radius in *local* (pre-scale) units
    uint32_t user;    // opaque id the caller can stash (entity handle etc)
} instance_desc;
typedef struct {
    uint32_t index;     // into batch->descs
    float    dist_sq;   // to camera, squared
    uint32_t sort_key;  // packed distance, filled by instance_sort
} instance_visible;
typedef int instance_mesh_id;
#define INSTANCING_BAD_MESH  (-1)
static inline float instancing_world_radius(const instance_desc *d) {
    float s = d->scale.x;
    if (d->scale.y > s) s = d->scale.y;
    if (d->scale.z > s) s = d->scale.z;
    return d->radius * s;
}
#endif
