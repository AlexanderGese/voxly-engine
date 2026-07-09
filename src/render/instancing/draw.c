#include "draw.h"
#include "../../util/darray.h"
#include "../../util/log.h"
static int draw_slice(instancing_mesh *m,
                      const instancing_gpu_instance *recs, int count) {
    int calls = 0;
    int done = 0;
    instancing_mesh_bind(m);

    while (done < count) {
        int chunk = count - done;
        if (chunk > INSTANCING_MAX_PER_BATCH) chunk = INSTANCING_MAX_PER_BATCH;

        // restream just this chunk into the mesh's per-instance vbo. the vbo
        // orphans itself, so each sub-batch gets a clean block.
        instancing_mesh_set_instances(m, recs + done, chunk);

        glDrawArraysInstanced(GL_TRIANGLES, 0, m->base_count, chunk);
        calls++;
        done += chunk;
    }

    return calls;
}

void instancing_draw_group(const instancing_batch *batch, int group_index,
                           instancing_registry *reg,
                           instancing_stats *stats) {
    if (group_index < 0 || group_index >= (int)darr_len(batch->groups)) return;
const instancing_batch_group *g = &batch->groups[group_index];
instancing_mesh *m = instancing_registry_get(reg, g->mesh_id);
int calls = draw_slice(m, batch->records + g->offset, g->count);
int base_tris = m->base_count / 3;
instancing_stats_add_batch(stats, g->count, base_tris, calls);
