#include "instancing.h"

#include "frustum_batch.h"
#include "draw.h"
#include "../../util/log.h"

void instancing_init(instancing *ix) {
    instancing_registry_init(&ix->registry);
    instancing_data_init(&ix->data);
    instancing_batch_init(&ix->batch);
    instancing_stats_reset(&ix->stats);
    ix->initialised = 1;
}

void instancing_destroy(instancing *ix) {
    if (!ix->initialised) return;
    instancing_batch_free(&ix->batch);
    instancing_data_free(&ix->data);
    instancing_registry_destroy(&ix->registry);
    ix->initialised = 0;
}

int instancing_register_mesh(instancing *ix, const char *name,
                             const instancing_base_vertex *verts, int count,
                             aabb box, int initial_instances) {
    return instancing_registry_add(&ix->registry, name, verts, count, box,
                                   initial_instances);
}

int instancing_add(instancing *ix, const instancing_instance *it) {
    // if the caller left the cull box empty, borrow the mesh's model-space box
    // so the frustum test has something sane to work with.
    instancing_instance rec = *it;
    if (rec.local_box.min.x == 0 && rec.local_box.max.x == 0 &&
        rec.local_box.min.y == 0 && rec.local_box.max.y == 0 &&
        rec.local_box.min.z == 0 && rec.local_box.max.z == 0) {
        instancing_mesh *m = instancing_registry_get(&ix->registry, rec.mesh_id);
        if (m) rec.local_box = m->local_box;
    }
    return instancing_data_add(&ix->data, &rec);
}

int instancing_update(instancing *ix, int handle,
                      const instancing_instance *it) {
    instancing_instance *dst = instancing_data_get(&ix->data, handle);
    if (!dst) return -1;
    uint32_t keep = dst->flags & INSTANCING_FLAG_ACTIVE;
    *dst = *it;
    dst->flags |= keep | INSTANCING_FLAG_DIRTY;  // never lose ACTIVE
    return 0;
}

void instancing_remove(instancing *ix, int handle) {
    instancing_data_remove(&ix->data, handle);
}

void instancing_begin_frame(instancing *ix) {
    instancing_stats_reset(&ix->stats);
    instancing_batch_reset(&ix->batch);
}

void instancing_cull(instancing *ix, const frustum *fr) {
    if (fr) {
        instancing_frustum_batch_build(&ix->data, fr, &ix->batch, &ix->stats);
    } else {
        // no frustum -> draw everything (shadow pass, etc).
        instancing_frustum_batch_build_all(&ix->data, &ix->batch, &ix->stats);
    }
}

void instancing_submit(instancing *ix) {
    if (!ix->initialised) {
        LOGW("instancing_submit: subsystem not initialised");
        return;
    }
    instancing_draw_batch(&ix->batch, &ix->registry, &ix->stats);
}

const instancing_stats *instancing_get_stats(const instancing *ix) {
    return &ix->stats;
}
