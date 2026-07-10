#include "lt_mesh.h"

#include "../../util/darray.h"

void lt_mesh_init(lt_mesh *m) {
    m->verts   = NULL;
    m->indices = NULL;
    m->quad_count  = 0;
    m->skirt_quads = 0;
    darr_reserve(m->verts,   LT_VERT_RESERVE);
    darr_reserve(m->indices, LT_IDX_RESERVE);
}

void lt_mesh_reset(lt_mesh *m) {
    darr_clear(m->verts);
    darr_clear(m->indices);
    m->quad_count  = 0;
    m->skirt_quads = 0;
}

void lt_mesh_free(lt_mesh *m) {
    darr_free(m->verts);
    darr_free(m->indices);
    m->quad_count  = 0;
    m->skirt_quads = 0;
}

void lt_mesh_quad(lt_mesh *m,
                  lt_vertex v0, lt_vertex v1, lt_vertex v2, lt_vertex v3,
                  int is_skirt) {
    uint32_t base = (uint32_t)darr_len(m->verts);

    darr_push(m->verts, v0);
    darr_push(m->verts, v1);
    darr_push(m->verts, v2);
    darr_push(m->verts, v3);

    // two tris, shared diagonal base..base+2. winding matches the engine's
    // ccw-front-face convention so backface culling keeps the right side.
    darr_push(m->indices, base + 0);
    darr_push(m->indices, base + 1);
    darr_push(m->indices, base + 2);
    darr_push(m->indices, base + 0);
    darr_push(m->indices, base + 2);
    darr_push(m->indices, base + 3);

    m->quad_count++;
    if (is_skirt) m->skirt_quads++;
}

int lt_mesh_tris(const lt_mesh *m) {
    return (int)(darr_len(m->indices) / 3);
}
