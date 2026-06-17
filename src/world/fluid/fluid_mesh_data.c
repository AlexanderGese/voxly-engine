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
