#include "draw_indirect.h"

#include "instance_buffer.h"
#include "../../util/darray.h"
#include "../../util/log.h"

#include <stddef.h>
#include <string.h>

// base vertex attribs, locations 0..2 — same as instance_mesh.c. duplicated
// because the merged vao wires a different base vbo.
static void setup_base_attribs(void) {
    const GLsizei stride = sizeof(instancing_base_vertex);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(instancing_base_vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(instancing_base_vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(instancing_base_vertex, light));
}

void instancing_indirect_init(instancing_indirect *ind) {
    memset(ind, 0, sizeof *ind);
    for (int i = 0; i < INSTANCING_MAX_MESHES; ++i) {
        ind->mesh_first[i] = -1;
        ind->mesh_count[i] = 0;
    }

    glGenVertexArrays(1, &ind->vao);
    glGenBuffers(1, &ind->base_vbo);
    glGenBuffers(1, &ind->inst_vbo);
    glGenBuffers(1, &ind->cmd_vbo);

    // seed each buffer with a small allocation so the first frame doesn't have
    // to special-case an empty handle.
    glBindVertexArray(ind->vao);

    glBindBuffer(GL_ARRAY_BUFFER, ind->base_vbo);
    glBufferData(GL_ARRAY_BUFFER, 64 * sizeof(instancing_base_vertex),
                 NULL, GL_STATIC_DRAW);
    setup_base_attribs();
    ind->base_capacity = 64;

    glBindBuffer(GL_ARRAY_BUFFER, ind->inst_vbo);
    glBufferData(GL_ARRAY_BUFFER, 64 * sizeof(instancing_gpu_instance),
                 NULL, GL_STREAM_DRAW);
    instancing_buffer_setup_attribs(3);
    ind->inst_capacity = 64;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ind->cmd_vbo);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, 16 * sizeof(instancing_draw_cmd),
                 NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    ind->cmd_capacity = 16;

    ind->initialised = 1;
}

void instancing_indirect_destroy(instancing_indirect *ind) {
    if (ind->vao) glDeleteVertexArrays(1, &ind->vao);
    if (ind->base_vbo) glDeleteBuffers(1, &ind->base_vbo);
    if (ind->inst_vbo) glDeleteBuffers(1, &ind->inst_vbo);
    if (ind->cmd_vbo) glDeleteBuffers(1, &ind->cmd_vbo);
    memset(ind, 0, sizeof *ind);
}

void instancing_indirect_pack_bases(instancing_indirect *ind,
                                    instancing_registry *reg) {
    if (!ind->initialised) return;

    // walk the registry, copy every mesh's base geometry back from its vbo
    // into one contiguous cpu array, remembering the offset of each. we read
    // it back via glGetBufferSubData — meshes own their static vbo, and we
    // don't keep a cpu copy around, so a readback is the honest way to merge.
    instancing_base_vertex *merged = NULL;

    for (int id = 0; id < INSTANCING_MAX_MESHES; ++id) {
        instancing_mesh *m = instancing_registry_get(reg, id);
        if (!m || m->base_count <= 0) {
            ind->mesh_first[id] = -1;
            ind->mesh_count[id] = 0;
            continue;
        }

        int first = (int)darr_len(merged);
        ind->mesh_first[id] = first;
        ind->mesh_count[id] = m->base_count;

        // make room then read straight into the slot.
        darr_reserve(merged, (size_t)m->base_count);
        glBindBuffer(GL_ARRAY_BUFFER, m->base_vbo);
        glGetBufferSubData(GL_ARRAY_BUFFER, 0,
                           (long)m->base_count * sizeof(instancing_base_vertex),
                           merged + first);
        // bump the darray length to cover what we just wrote.
        darr_hdr(merged)->len += (size_t)m->base_count;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int total = (int)darr_len(merged);
    if (total <= 0) { darr_free(merged); return; }

    // upload the merged base vbo, growing if needed.
    glBindBuffer(GL_ARRAY_BUFFER, ind->base_vbo);
    if (total > ind->base_capacity) {
        int cap = ind->base_capacity;
        while (cap < total) cap *= 2;
        glBufferData(GL_ARRAY_BUFFER,
                     (long)cap * sizeof(instancing_base_vertex),
                     NULL, GL_STATIC_DRAW);
        ind->base_capacity = cap;
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (long)total * sizeof(instancing_base_vertex), merged);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    darr_free(merged);
}

void instancing_indirect_submit(instancing_indirect *ind,
                                const instancing_batch *batch,
                                instancing_stats *stats) {
    if (!ind->initialised) {
        LOGW("instancing_indirect_submit: not initialised");
        return;
    }
    int ng = (int)darr_len(batch->groups);
    if (ng == 0) { ind->ncommands = 0; return; }

    // the batch already laid records out grouped + contiguous by mesh, and the
    // groups are in mesh-id order. that means group offsets line up exactly
    // with base_instance, so we can upload batch->records verbatim. tidy.
    int total_records = (int)darr_len(batch->records);

    glBindBuffer(GL_ARRAY_BUFFER, ind->inst_vbo);
    if (total_records > ind->inst_capacity) {
        int cap = ind->inst_capacity;
        while (cap < total_records) cap *= 2;
        glBufferData(GL_ARRAY_BUFFER,
                     (long)cap * sizeof(instancing_gpu_instance),
                     NULL, GL_STREAM_DRAW);
        ind->inst_capacity = cap;
    } else {
        glBufferData(GL_ARRAY_BUFFER,
                     (long)ind->inst_capacity * sizeof(instancing_gpu_instance),
                     NULL, GL_STREAM_DRAW);
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (long)total_records * sizeof(instancing_gpu_instance),
                    batch->records);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // build one command per group whose mesh got packed. groups referencing an
    // unpacked mesh are skipped (and warned once) rather than drawing junk.
    instancing_draw_cmd *cmds = NULL;
    for (int i = 0; i < ng; ++i) {
        const instancing_batch_group *g = &batch->groups[i];
        if (g->mesh_id < 0 || g->mesh_id >= INSTANCING_MAX_MESHES) continue;
        int first = ind->mesh_first[g->mesh_id];
        int vcount = ind->mesh_count[g->mesh_id];
        if (first < 0 || vcount <= 0) {
            LOGW("indirect: mesh %d not packed, skipping group", g->mesh_id);
            continue;
        }
        instancing_draw_cmd c;
        c.count          = (GLuint)vcount;
        c.instance_count = (GLuint)g->count;
        c.first          = (GLuint)first;
        c.base_instance  = (GLuint)g->offset;
        darr_push(cmds, c);

        int base_tris = vcount / 3;
        instancing_stats_add_batch(stats, g->count, base_tris, 0);
    }

    int ncmd = (int)darr_len(cmds);
    if (ncmd == 0) { darr_free(cmds); ind->ncommands = 0; return; }

    // upload the command buffer.
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ind->cmd_vbo);
    if (ncmd > ind->cmd_capacity) {
        int cap = ind->cmd_capacity;
        while (cap < ncmd) cap *= 2;
        glBufferData(GL_DRAW_INDIRECT_BUFFER,
                     (long)cap * sizeof(instancing_draw_cmd),
                     NULL, GL_STREAM_DRAW);
        ind->cmd_capacity = cap;
    }
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0,
                    (long)ncmd * sizeof(instancing_draw_cmd), cmds);

    // one call, the whole batch. base_instance in each command picks the right
    // slice of the merged instance vbo per group.
    glBindVertexArray(ind->vao);
    glMultiDrawArraysIndirect(GL_TRIANGLES, (const void*)0, ncmd,
                              sizeof(instancing_draw_cmd));
    glBindVertexArray(0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    // one multi-draw == one logical draw call from the cpu's view.
    stats->draw_calls += 1;
    ind->ncommands = ncmd;
    darr_free(cmds);
}
