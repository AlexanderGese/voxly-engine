#include "instance_mesh.h"
#include "../../util/log.h"
#include <stddef.h>
#include <string.h>
// static geometry attribs, locations 0..2. same layout as render/mesh.c so a
// shader can be shared between the plain and instanced paths.
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

void instancing_mesh_init(instancing_mesh *m,
                          const instancing_base_vertex *verts, int count,
                          aabb box, int initial_instances) {
    memset(m, 0, sizeof *m);
m->local_box = box;
m->base_count = count;
glGenVertexArrays(1, &m->vao);
glGenBuffers(1, &m->base_vbo);
glBindVertexArray(m->vao);
// upload the static base geometry once.
glBindBuffer(GL_ARRAY_BUFFER, m->base_vbo);
glBufferData(GL_ARRAY_BUFFER,
                 (long)count * sizeof(instancing_base_vertex),
                 verts, GL_STATIC_DRAW);
setup_base_attribs();
// now the per-instance buffer. its attribs share the same vao but read
// from a different vbo with divisor 1. locations continue at 3.
instancing_buffer_init(&m->inst, initial_instances);
glBindBuffer(GL_ARRAY_BUFFER, m->inst.vbo);
instancing_buffer_setup_attribs(3);
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);
m->uploaded = (verts != NULL && count > 0);
if (!m->uploaded)
        LOGW("instancing_mesh_init: base mesh has no geometry");
}

void instancing_mesh_destroy(instancing_mesh *m) {
    instancing_buffer_destroy(&m->inst);
    if (m->base_vbo) glDeleteBuffers(1, &m->base_vbo);
    if (m->vao) glDeleteVertexArrays(1, &m->vao);
    m->vao = m->base_vbo = 0;
    m->base_count = 0;
    m->uploaded = 0;
}

void instancing_mesh_set_instances(instancing_mesh *m,
                                   const instancing_gpu_instance *recs,
                                   int count) {
    instancing_buffer_upload(&m->inst, recs, count);
}

void instancing_mesh_bind(const instancing_mesh *m) {
    glBindVertexArray(m->vao);
}

int instancing_mesh_instance_count(const instancing_mesh *m) {
    return m->inst.count;
}
