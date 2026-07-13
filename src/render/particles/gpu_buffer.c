#include "gpu_buffer.h"

#include "../../util/log.h"

#include <stddef.h>

// vertex attribute layout. mirrors particles_vertex:
// loc 0: position (vec3)
// loc 1: uv       (vec2)
// loc 2: color    (vec4)
static void setup_attribs(void) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(particles_vertex),
                          (void*)offsetof(particles_vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(particles_vertex),
                          (void*)offsetof(particles_vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(particles_vertex),
                          (void*)offsetof(particles_vertex, r));
}

void particles_gpu_init(particles_gpu_buffer *g, int initial_verts) {
    if (initial_verts < 64) initial_verts = 64;

    glGenVertexArrays(1, &g->vao);
    glGenBuffers(1, &g->vbo);

    glBindVertexArray(g->vao);
    glBindBuffer(GL_ARRAY_BUFFER, g->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (long)initial_verts * sizeof(particles_vertex),
                 NULL, GL_STREAM_DRAW);
    setup_attribs();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    g->capacity_verts = initial_verts;
    g->vert_count     = 0;
    g->initialised    = 1;
}

void particles_gpu_destroy(particles_gpu_buffer *g) {
    if (g->vao) glDeleteVertexArrays(1, &g->vao);
    if (g->vbo) glDeleteBuffers(1, &g->vbo);
    g->vao = g->vbo = 0;
    g->capacity_verts = 0;
    g->vert_count = 0;
    g->initialised = 0;
}

void particles_gpu_upload(particles_gpu_buffer *g,
                          const particles_vertex *verts, int count) {
    if (!g->initialised) {
        LOGW("particles_gpu_upload: buffer not initialised");
        return;
    }
    g->vert_count = count;
    if (count <= 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, g->vbo);

    if (count > g->capacity_verts) {
        // grow with headroom so we're not reallocating every frame the
        // particle count creeps up. orphan-and-resize in one shot.
        int newcap = g->capacity_verts;
        while (newcap < count) newcap *= 2;
        glBufferData(GL_ARRAY_BUFFER,
                     (long)newcap * sizeof(particles_vertex),
                     NULL, GL_STREAM_DRAW);
        g->capacity_verts = newcap;
    } else {
        // orphan the old storage so the driver hands us a fresh block and we
        // don't stall waiting on the previous frame's draw. classic trick.
        glBufferData(GL_ARRAY_BUFFER,
                     (long)g->capacity_verts * sizeof(particles_vertex),
                     NULL, GL_STREAM_DRAW);
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (long)count * sizeof(particles_vertex), verts);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void particles_gpu_draw(const particles_gpu_buffer *g) {
    if (!g->initialised || g->vert_count <= 0) return;
    glBindVertexArray(g->vao);
    glDrawArrays(GL_TRIANGLES, 0, g->vert_count);
    glBindVertexArray(0);
}
