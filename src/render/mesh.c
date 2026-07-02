#include "mesh.h"
#include "../util/log.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void mesh_init(mesh *m) {
    memset(m, 0, sizeof *m);
    glGenVertexArrays(1, &m->vao);
    glGenBuffers(1, &m->vbo);

    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*)offsetof(vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*)offsetof(vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void*)offsetof(vertex, light));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void mesh_reset(mesh *m) {
    m->count = 0;
    m->uploaded = 0;
}

void mesh_push_vertex(mesh *m, vertex v) {
    if (m->count == m->cap) {
        int nc = m->cap ? m->cap * 2 : 1024;
        vertex *nv = realloc(m->verts, nc * sizeof(vertex));
        if (!nv) { LOGE("mesh_push_vertex OOM"); return; }
        m->verts = nv;
        m->cap   = nc;
    }
    m->verts[m->count++] = v;
}

void mesh_upload(mesh *m) {
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 m->count * sizeof(vertex),
                 m->verts,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m->uploaded = 1;
}

void mesh_draw(const mesh *m) {
    if (!m->uploaded || m->count == 0) return;
    glBindVertexArray(m->vao);
    glDrawArrays(GL_TRIANGLES, 0, m->count);
}

void mesh_destroy(mesh *m) {
    if (m->vao) glDeleteVertexArrays(1, &m->vao);
    if (m->vbo) glDeleteBuffers(1, &m->vbo);
    free(m->verts);
    memset(m, 0, sizeof *m);
}
