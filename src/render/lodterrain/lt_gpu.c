#include "lt_gpu.h"
#include "../../util/darray.h"
#include <stddef.h>
void lt_gpu_init(lt_gpu_mesh *gm) {
    gm->vao = gm->vbo = gm->ebo = 0;
    gm->index_count = 0;
    gm->uploaded = 0;
}

static void ensure_objects(lt_gpu_mesh *gm) {
    if (gm->vao) return;
glGenVertexArrays(1, &gm->vao);
glGenBuffers(1, &gm->vbo);
glGenBuffers(1, &gm->ebo);
glBindVertexArray(gm->vao);
glBindBuffer(GL_ARRAY_BUFFER, gm->vbo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gm->ebo);
// layout matches lt_vertex == render/mesh.h vertex: pos(3) uv(2) light(1).
GLsizei stride = (GLsizei)sizeof(lt_vertex);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(lt_vertex, x));
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(lt_vertex, u));
glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(lt_vertex, light));
glBindVertexArray(0);
}

void lt_gpu_upload(lt_gpu_mesh *gm, const lt_mesh *m) {
    size_t vcount = darr_len(m->verts);
    size_t icount = darr_len(m->indices);

    if (vcount == 0 || icount == 0) {
        // empty mesh: keep any existing objects but draw nothing. cheaper than
        // tearing down + recreating when a chunk flickers empty for a frame.
        gm->index_count = 0;
        gm->uploaded = 1;
        return;
    }

    ensure_objects(gm);

    glBindVertexArray(gm->vao);

    glBindBuffer(GL_ARRAY_BUFFER, gm->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(vcount * sizeof(lt_vertex)),
                 m->verts, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gm->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(icount * sizeof(uint32_t)),
                 m->indices, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

    gm->index_count = (int)icount;
    gm->uploaded = 1;
}

void lt_gpu_draw(const lt_gpu_mesh *gm) {
    if (!gm->uploaded || gm->index_count == 0 || gm->vao == 0) return;
glBindVertexArray(gm->vao);
glDrawElements(GL_TRIANGLES, gm->index_count, GL_UNSIGNED_INT, 0);
glBindVertexArray(0);
}

void lt_gpu_free(lt_gpu_mesh *gm) {
    if (gm->ebo) glDeleteBuffers(1, &gm->ebo);
    if (gm->vbo) glDeleteBuffers(1, &gm->vbo);
    if (gm->vao) glDeleteVertexArrays(1, &gm->vao);
    lt_gpu_init(gm);
}
