#ifndef RENDER_INSTANCING_REGISTRY_H
#define RENDER_INSTANCING_REGISTRY_H
// maps mesh ids to instancing_mesh objects. flat array indexed by id, because
// ids are small and dense (handed out sequentially on register). nothing here
// is thread safe — register at load time, draw on the render thread.
#include "instance_mesh.h"
typedef struct {
    instancing_mesh meshes[INSTANCING_MAX_MESHES];
    char            names[INSTANCING_MAX_MESHES][32];
    int             used[INSTANCING_MAX_MESHES];   // slot occupied flag
    int             count;                          // highest id + 1, roughly
} instancing_registry;
void instancing_registry_init(instancing_registry *r);
void instancing_registry_destroy(instancing_registry *r);
// register a base mesh under `name`. returns the assigned id, or
// INSTANCING_MESH_NONE if full. the registry takes ownership of the gl
// objects created for the mesh.
int  instancing_registry_add(instancing_registry *r, const char *name,
                             const instancing_base_vertex *verts, int count,
                             aabb box, int initial_instances);
int  instancing_registry_find(const instancing_registry *r, const char *name);
instancing_mesh *instancing_registry_get(instancing_registry *r, int id);
#endif
