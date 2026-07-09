#include "draw_indirect.h"
#include "instance_buffer.h"
#include "../../util/darray.h"
#include "../../util/log.h"
#include <stddef.h>
#include <string.h>
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
for (int i = 0;
i < INSTANCING_MAX_MESHES;
++i) {
        ind->mesh_first[i] = -1;
        ind->mesh_count[i] = 0;
    }

    glGenVertexArrays(1, &ind->vao);
glGenBuffers(1, &ind->base_vbo);
glGenBuffers(1, &ind->inst_vbo);
glGenBuffers(1, &ind->cmd_vbo);
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
instancing_base_vertex *merged = NULL;
for (int id = 0;
id < INSTANCING_MAX_MESHES;
int total = (int)darr_len(merged);
glBindBuffer(GL_ARRAY_BUFFER, 0);
darr_free(merged);
