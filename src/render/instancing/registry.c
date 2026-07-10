#include "registry.h"

#include "../../util/log.h"

#include <string.h>

void instancing_registry_init(instancing_registry *r) {
    memset(r, 0, sizeof *r);
}

void instancing_registry_destroy(instancing_registry *r) {
    for (int i = 0; i < INSTANCING_MAX_MESHES; ++i) {
        if (r->used[i]) instancing_mesh_destroy(&r->meshes[i]);
    }
    memset(r, 0, sizeof *r);
}

int instancing_registry_add(instancing_registry *r, const char *name,
                            const instancing_base_vertex *verts, int count,
                            aabb box, int initial_instances) {
    // reuse a name collision rather than leak a second copy. organic engines
    // re-register on hot reload all the time and i got tired of the leak.
    int existing = instancing_registry_find(r, name);
    if (existing != INSTANCING_MESH_NONE) {
        instancing_mesh_destroy(&r->meshes[existing]);
        instancing_mesh_init(&r->meshes[existing], verts, count, box,
                             initial_instances);
        return existing;
    }

    // find a free slot.
    int id = INSTANCING_MESH_NONE;
    for (int i = 0; i < INSTANCING_MAX_MESHES; ++i) {
        if (!r->used[i]) { id = i; break; }
    }
    if (id == INSTANCING_MESH_NONE) {
        LOGE("instancing_registry_add: registry full (%d meshes)",
             INSTANCING_MAX_MESHES);
        return INSTANCING_MESH_NONE;
    }

    instancing_mesh_init(&r->meshes[id], verts, count, box, initial_instances);
    r->used[id] = 1;

    // truncating copy; names are short labels, not user content.
    size_t n = sizeof r->names[id] - 1;
    strncpy(r->names[id], name ? name : "", n);
    r->names[id][n] = '\0';

    if (id + 1 > r->count) r->count = id + 1;
    return id;
}

int instancing_registry_find(const instancing_registry *r, const char *name) {
    if (!name) return INSTANCING_MESH_NONE;
    for (int i = 0; i < INSTANCING_MAX_MESHES; ++i) {
        if (r->used[i] && strcmp(r->names[i], name) == 0) return i;
    }
    return INSTANCING_MESH_NONE;
}

instancing_mesh *instancing_registry_get(instancing_registry *r, int id) {
    if (id < 0 || id >= INSTANCING_MAX_MESHES) return NULL;
    if (!r->used[id]) return NULL;
    return &r->meshes[id];
}
