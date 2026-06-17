#include "fluid_mesh_data.h"
#include "fluid_level.h"
#include "flow_direction.h"

#include <stdlib.h>
#include <string.h>

void voxl_fluid_mesh_init(voxl_fluid_mesh *m) {
    if (!m) return;
    m->verts = NULL;
    m->count = 0;
    m->cap = 0;
}

void voxl_fluid_mesh_destroy(voxl_fluid_mesh *m) {
    if (!m) return;
    free(m->verts);
    m->verts = NULL;
    m->count = 0;
    m->cap = 0;
}

void voxl_fluid_mesh_reset(voxl_fluid_mesh *m) {
    if (!m) return;
    m->count = 0;   // keep the buffer, just rewind
}

bool voxl_fluid_mesh_push(voxl_fluid_mesh *m, voxl_fluid_vertex v) {
    if (!m) return false;
    if (m->count >= m->cap) {
        int ncap = m->cap ? m->cap * 2 : 64;
        voxl_fluid_vertex *nv = realloc(m->verts, (size_t)ncap * sizeof(*nv));
        if (!nv) return false;
        m->verts = nv;
        m->cap = ncap;
    }
    m->verts[m->count++] = v;
    return true;
}

// sample a cell's surface height, treating missing/empty as floor level y.
static float voxl_fluid_sample_h(const voxl_fluid_grid *g, int x, int y, int z) {
    const voxl_fluid_cell *c = voxl_fluid_at_const(g, x, y, z);
    if (!c || voxl_fluid_cell_empty(c)) return (float)y;
    return voxl_fluid_surface_height(c, y);
}

// average the four cells meeting at a grid corner to get a smooth height.
static float voxl_fluid_corner(const voxl_fluid_grid *g, int cx, int y, int cz) {
    float a = voxl_fluid_sample_h(g, cx,     y, cz);
    float b = voxl_fluid_sample_h(g, cx - 1, y, cz);
    float c = voxl_fluid_sample_h(g, cx,     y, cz - 1);
    float d = voxl_fluid_sample_h(g, cx - 1, y, cz - 1);
    return voxl_fluid_corner_height(a, b, c, d);
}

int voxl_fluid_mesh_build(voxl_fluid_mesh *m, const voxl_fluid_grid *g) {
    if (!m || !g) return 0;
    voxl_fluid_mesh_reset(m);

    for (int y = 0; y < VOXL_FLUID_GRID_N; y++) {
        for (int z = 0; z < VOXL_FLUID_GRID_N; z++) {
            for (int x = 0; x < VOXL_FLUID_GRID_N; x++) {
                const voxl_fluid_cell *c = voxl_fluid_at_const(g, x, y, z);
                if (!c || voxl_fluid_cell_empty(c)) continue;

                // only draw a top if the cell above isnt also fluid
                const voxl_fluid_cell *up = voxl_fluid_at_const(g, x, y + 1, z);
                if (up && !voxl_fluid_cell_empty(up)) continue;

                // four corner heights of this cell's top face
                float h00 = voxl_fluid_corner(g, x,     y, z);
                float h10 = voxl_fluid_corner(g, x + 1, y, z);
                float h11 = voxl_fluid_corner(g, x + 1, y, z + 1);
                float h01 = voxl_fluid_corner(g, x,     y, z + 1);

                voxl_fluid_flow fl = voxl_fluid_flow_normalize(
                    voxl_fluid_flow_at(g, x, y, z));

                voxl_fluid_vertex v00 = { (float)x,     h00, (float)z,     0,0, fl.dx, fl.dz };
                voxl_fluid_vertex v10 = { (float)x + 1, h10, (float)z,     1,0, fl.dx, fl.dz };
                voxl_fluid_vertex v11 = { (float)x + 1, h11, (float)z + 1, 1,1, fl.dx, fl.dz };
                voxl_fluid_vertex v01 = { (float)x,     h01, (float)z + 1, 0,1, fl.dx, fl.dz };

                // two triangles, ccw
                voxl_fluid_mesh_push(m, v00);
                voxl_fluid_mesh_push(m, v10);
                voxl_fluid_mesh_push(m, v11);
                voxl_fluid_mesh_push(m, v00);
                voxl_fluid_mesh_push(m, v11);
                voxl_fluid_mesh_push(m, v01);
            }
        }
    }
    return m->count;
}
