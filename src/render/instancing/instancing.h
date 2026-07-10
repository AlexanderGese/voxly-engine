#ifndef RENDER_INSTANCING_INSTANCING_H
#define RENDER_INSTANCING_INSTANCING_H

// top-level facade for the instanced draw subsystem. owns the registry, the
// cpu instance store, the reusable batch and the per-frame stats. the renderer
// holds one of these. usage per frame:
//
// instancing_begin_frame(&ix);
// instancing_cull(&ix, &frustum);     // build + cull + batch
// ... bind shader, set view/proj, bind atlas ...
// instancing_submit(&ix);             // issue the draw calls
//
// registration of base meshes and adding/removing instances happens outside
// the per-frame loop (at load, on chunk decorate, on prop spawn, etc).

#include "../../math/frustum.h"
#include "registry.h"
#include "instance_data.h"
#include "batch.h"
#include "instancing_stats.h"

typedef struct {
    instancing_registry      registry;
    instancing_instance_data data;
    instancing_batch         batch;
    instancing_stats         stats;
    int                      initialised;
} instancing;

void instancing_init(instancing *ix);
void instancing_destroy(instancing *ix);

// --- registration / scene editing (off the hot path) -----------------------

// register a base mesh. thin wrapper over the registry; returns the mesh id.
int  instancing_register_mesh(instancing *ix, const char *name,
                              const instancing_base_vertex *verts, int count,
                              aabb box, int initial_instances);

// add an instance to the scene. returns a stable handle.
int  instancing_add(instancing *ix, const instancing_instance *it);

// move/recolor an existing instance. returns 0 on success, -1 on bad handle.
int  instancing_update(instancing *ix, int handle,
                       const instancing_instance *it);

void instancing_remove(instancing *ix, int handle);

// --- per-frame --------------------------------------------------------------

// reset stats + batch for a new frame.
void instancing_begin_frame(instancing *ix);

// cull against the frustum and build the per-mesh batches. pass NULL to skip
// culling (draw everything — handy for shadow passes).
void instancing_cull(instancing *ix, const frustum *fr);

// issue the draw calls for the built batch. caller has the shader/atlas bound.
void instancing_submit(instancing *ix);

// access the frame's stats (for the F3 overlay).
const instancing_stats *instancing_get_stats(const instancing *ix);

#endif
