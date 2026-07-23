#include "water_surface_mesh.h"
#include "../../util/darray.h"

#include <math.h>

int water_surface_mesh_create(water_surface_mesh *m, int res, float cell) {
    m->res   = res < 1 ? 1 : res;
    m->cell  = cell;
    m->verts = NULL;
    m->idx   = NULL;
    m->index_count = 0;
    m->origin_x = INFINITY;   // force the first update to rebuild
    m->origin_z = INFINITY;

    glGenVertexArrays(1, &m->vao);
    glGenBuffers(1, &m->vbo);
    glGenBuffers(1, &m->ibo);

    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);

    size_t stride = sizeof(water_surface_vertex);
    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(water_surface_vertex, x));
    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(water_surface_vertex, nx));
    glEnableVertexAttribArray(2); // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(water_surface_vertex, u));
    glBindVertexArray(0);

    // indices are constant for a fixed grid, build them once
    int n = m->res;
    for (int z = 0; z < n; z++) {
        for (int x = 0; x < n; x++) {
            unsigned int a = (unsigned)(z * (n + 1) + x);
            unsigned int b = a + 1;
            unsigned int c = a + (unsigned)(n + 1);
            unsigned int d = c + 1;
            // two tris per cell, ccw
            darr_push(m->idx, a); darr_push(m->idx, c); darr_push(m->idx, b);
            darr_push(m->idx, b); darr_push(m->idx, c); darr_push(m->idx, d);
        }
    }
    m->index_count = (int)darr_len(m->idx);

    glBindVertexArray(m->vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (long)(darr_len(m->idx) * sizeof(unsigned int)),
                 m->idx, GL_STATIC_DRAW);
    glBindVertexArray(0);
    return 1;
}

void water_surface_mesh_destroy(water_surface_mesh *m) {
    if (m->vao) glDeleteVertexArrays(1, &m->vao);
    if (m->vbo) glDeleteBuffers(1, &m->vbo);
    if (m->ibo) glDeleteBuffers(1, &m->ibo);
    darr_free(m->verts);
    darr_free(m->idx);
    m->vao = m->vbo = m->ibo = 0;
}

int water_surface_mesh_update(water_surface_mesh *m, const water_wave_field *f,
                              float cam_x, float cam_z, float surface_y) {
    // snap the grid origin so it tracks the camera in whole cells. the mesh is
    // centered on the camera, extending res/2 cells each way.
    float half = m->res * 0.5f * m->cell;
    float ox = floorf((cam_x - half) / m->cell) * m->cell;
    float oz = floorf((cam_z - half) / m->cell) * m->cell;

    // also rebuild whenever waves animate — but the caller calls this every
    // frame anyway, so just check the snap to avoid pointless reuploads when
    // the camera is still AND time hasnt moved enough. we rebuild on any move.
    if (ox == m->origin_x && oz == m->origin_z && f->time == 0.0f)
        return 0;

    m->origin_x = ox;
    m->origin_z = oz;

    darr_clear(m->verts);
    int n = m->res;
    for (int z = 0; z <= n; z++) {
        for (int x = 0; x <= n; x++) {
            float wx = ox + (float)x * m->cell;
            float wz = oz + (float)z * m->cell;

            water_wave_sample s =
                water_wave_field_sample(f, wx, wz, surface_y);

            water_surface_vertex v;
            v.x = wx + s.offset.x;
            v.y = surface_y + s.offset.y;
            v.z = wz + s.offset.z;
            v.nx = s.normal.x;
            v.ny = s.normal.y;
            v.nz = s.normal.z;
            // uv tied to world space so the ripple texture doesnt swim when
            // the grid re-snaps. scaled down so one tile spans a few blocks.
            v.u = wx * 0.0625f;
            v.v = wz * 0.0625f;
            darr_push(m->verts, v);
        }
    }

    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (long)(darr_len(m->verts) * sizeof(water_surface_vertex)),
                 m->verts, GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
    return 1;
}

void water_surface_mesh_draw(const water_surface_mesh *m) {
    if (m->index_count == 0) return;
    glBindVertexArray(m->vao);
    glDrawElements(GL_TRIANGLES, m->index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
