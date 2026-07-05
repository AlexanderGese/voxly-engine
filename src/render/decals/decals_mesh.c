#include "decals_mesh.h"
#include "decals_config.h"
#include "../../util/log.h"

#include <stddef.h>

// unit cube spanning [-0.5,0.5]^3, 12 tris, ccw winding. we draw it with
// culling off (the camera can sit inside the projector volume), so winding is
// academic, but keep it consistent in case someone flips a state somewhere.
static const float k_cube[] = {
    // -x face
    -0.5f,-0.5f,-0.5f,  -0.5f,-0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,
    -0.5f,-0.5f,-0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f,-0.5f,
    // +x face
     0.5f,-0.5f,-0.5f,   0.5f, 0.5f, 0.5f,   0.5f,-0.5f, 0.5f,
     0.5f,-0.5f,-0.5f,   0.5f, 0.5f,-0.5f,   0.5f, 0.5f, 0.5f,
    // -y face
    -0.5f,-0.5f,-0.5f,   0.5f,-0.5f,-0.5f,   0.5f,-0.5f, 0.5f,
    -0.5f,-0.5f,-0.5f,   0.5f,-0.5f, 0.5f,  -0.5f,-0.5f, 0.5f,
    // +y face
    -0.5f, 0.5f,-0.5f,   0.5f, 0.5f, 0.5f,   0.5f, 0.5f,-0.5f,
    -0.5f, 0.5f,-0.5f,  -0.5f, 0.5f, 0.5f,   0.5f, 0.5f, 0.5f,
    // -z face
    -0.5f,-0.5f,-0.5f,  -0.5f, 0.5f,-0.5f,   0.5f, 0.5f,-0.5f,
    -0.5f,-0.5f,-0.5f,   0.5f, 0.5f,-0.5f,   0.5f,-0.5f,-0.5f,
    // +z face
    -0.5f,-0.5f, 0.5f,   0.5f,-0.5f, 0.5f,   0.5f, 0.5f, 0.5f,
    -0.5f,-0.5f, 0.5f,   0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,
};

// wire the four columns of a mat4 instance attribute. a mat4 vertex attribute
// is four consecutive vec4 locations, each advancing once per instance.
static void setup_mat4_attr(int loc0, size_t base_off, size_t stride) {
    for (int c = 0; c < 4; c++) {
        glEnableVertexAttribArray(loc0 + c);
        glVertexAttribPointer(loc0 + c, 4, GL_FLOAT, GL_FALSE, (GLsizei)stride,
                              (void*)(base_off + (size_t)c * 4 * sizeof(float)));
        glVertexAttribDivisor(loc0 + c, 1);
    }
}

int decals_mesh_create(decals_mesh *m) {
    m->inst_cap = DECALS_BATCH_MAX;

    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);

    glGenBuffers(1, &m->cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m->cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof k_cube, k_cube, GL_STATIC_DRAW);
    // location 0: cube corner position, per-vertex.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &m->inst_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m->inst_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(sizeof(decals_mesh_inst) * m->inst_cap),
                 NULL, GL_STREAM_DRAW);

    size_t stride = sizeof(decals_mesh_inst);
    setup_mat4_attr(1, offsetof(decals_mesh_inst, model),     stride);  // 1..4
    setup_mat4_attr(5, offsetof(decals_mesh_inst, inv_model), stride);  // 5..8

    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, (GLsizei)stride,
                          (void*)offsetof(decals_mesh_inst, uv_rect));
    glVertexAttribDivisor(9, 1);

    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, (GLsizei)stride,
                          (void*)offsetof(decals_mesh_inst, params));
    glVertexAttribDivisor(10, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!m->vao || !m->cube_vbo || !m->inst_vbo) {
        LOGE("decals: failed to create mesh buffers");
        return 0;
    }
    return 1;
}

void decals_mesh_destroy(decals_mesh *m) {
    if (m->inst_vbo) glDeleteBuffers(1, &m->inst_vbo);
    if (m->cube_vbo) glDeleteBuffers(1, &m->cube_vbo);
    if (m->vao)      glDeleteVertexArrays(1, &m->vao);
    m->inst_vbo = m->cube_vbo = m->vao = 0;
}

void decals_mesh_upload(decals_mesh *m, const decals_mesh_inst *insts, int count) {
    if (count > m->inst_cap) count = m->inst_cap;
    if (count <= 0) return;
    glBindBuffer(GL_ARRAY_BUFFER, m->inst_vbo);
    // orphan first: hand the driver a fresh block so it doesnt wait on the
    // in-flight draw that still references last frame's contents.
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(sizeof(decals_mesh_inst) * m->inst_cap),
                 NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (GLsizeiptr)(sizeof(decals_mesh_inst) * count), insts);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void decals_mesh_draw(const decals_mesh *m, int count) {
    if (count <= 0) return;
    glBindVertexArray(m->vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, count);
    glBindVertexArray(0);
}
