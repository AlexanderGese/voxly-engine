#include "frustum_batch.h"

#include "transform.h"
#include "../../config.h"

// bake one instance and hand it to the batch under its mesh id.
static void stage_one(instancing_batch *batch, const instancing_instance *it) {
    instancing_gpu_instance rec;
    instancing_xform_bake(it, &rec);
    instancing_batch_stage(batch, it->mesh_id, &rec);
}

void instancing_frustum_batch_build(instancing_instance_data *data,
                                    const frustum *fr,
                                    instancing_batch *batch,
                                    instancing_stats *stats) {
#ifdef DEBUG_DISABLE_CULLING
    // honor the global debug toggle: skip the frustum test, draw it all.
    (void)fr;
    instancing_frustum_batch_build_all(data, batch, stats);
    return;
#else
    instancing_batch_reset(batch);

    int cap = instancing_data_capacity(data);
    for (int h = 0; h < cap; ++h) {
        instancing_instance *it = instancing_data_get(data, h);
        if (!it) continue;            // freed slot
        if (it->mesh_id < 0) continue; // no mesh, nothing to draw

        stats->instances_total++;

        // NO_CULL props (e.g. attached to the player) always pass.
        if (it->flags & INSTANCING_FLAG_NO_CULL) {
            stage_one(batch, it);
            continue;
        }

        aabb wb = instancing_xform_world_box(it);
        if (!frustum_contains_aabb(fr, wb)) {
            stats->instances_culled++;
            continue;
        }
        stage_one(batch, it);
    }

    instancing_batch_build(batch);
#endif
}

void instancing_frustum_batch_build_all(instancing_instance_data *data,
                                        instancing_batch *batch,
                                        instancing_stats *stats) {
    instancing_batch_reset(batch);

    int cap = instancing_data_capacity(data);
    for (int h = 0; h < cap; ++h) {
        instancing_instance *it = instancing_data_get(data, h);
        if (!it) continue;
        if (it->mesh_id < 0) continue;
        stats->instances_total++;
        stage_one(batch, it);
    }

    instancing_batch_build(batch);
}
