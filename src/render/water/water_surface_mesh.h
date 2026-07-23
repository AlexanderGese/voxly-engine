#ifndef RENDER_WATER_WATER_SURFACE_MESH_H
#define RENDER_WATER_WATER_SURFACE_MESH_H
#include "../gl.h"
#include "water_wave.h"
typedef struct {
    float x, y, z;      // displaced world position
    float nx, ny, nz;   // surface normal from the wave field
    float u, v;         // world-derived uv for the ripple/foam textures
} water_surface_vertex;
typedef struct {
    glid vao;
    glid vbo;
    glid ibo;
    int  index_count;
    int  res;           // cells per side
    float cell;         // cell size in blocks
    float origin_x;     // last snapped grid origin
    float origin_z;
    water_surface_vertex *verts;  // darray
    unsigned int        *idx;     // darray
} water_surface_mesh;
int  water_surface_mesh_create(water_surface_mesh *m, int res, float cell);
void water_surface_mesh_destroy(water_surface_mesh *m);
int  water_surface_mesh_update(water_surface_mesh *m, const water_wave_field *f,
                               float cam_x, float cam_z, float surface_y);
void water_surface_mesh_draw(const water_surface_mesh *m);
#endif
