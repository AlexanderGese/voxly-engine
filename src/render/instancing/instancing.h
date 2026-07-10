#ifndef RENDER_INSTANCING_INSTANCING_H
#define RENDER_INSTANCING_INSTANCING_H
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
int  instancing_register_mesh(instancing *ix, const char *name,
                              const instancing_base_vertex *verts, int count,
                              aabb box, int initial_instances);
int  instancing_add(instancing *ix, const instancing_instance *it);
int  instancing_update(instancing *ix, int handle,
                       const instancing_instance *it);
void instancing_remove(instancing *ix, int handle);
void instancing_begin_frame(instancing *ix);
void instancing_cull(instancing *ix, const frustum *fr);
void instancing_submit(instancing *ix);
#endif
