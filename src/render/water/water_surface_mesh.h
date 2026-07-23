#ifndef RENDER_WATER_WATER_SURFACE_MESH_H
#define RENDER_WATER_WATER_SURFACE_MESH_H

#include "../gl.h"
#include "water_wave.h"

// a tessellated grid of the water surface around the camera. we build it on the
// cpu so the wave field and the mesh always agree, then upload an interleaved
// vbo. the grid follows the camera (snapped to cell size) so it never runs out
// under you. cheaper than meshing every water block individually and it tiles
// the wave detail nicely.

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

// create an res x res grid of cells, each `cell` blocks wide.
int  water_surface_mesh_create(water_surface_mesh *m, int res, float cell);
void water_surface_mesh_destroy(water_surface_mesh *m);

// rebuild the vertex grid around (cam_x, cam_z) at surface_y, displaced by the
// wave field. snaps the origin to the cell grid so it only re-uploads when the
// camera crosses a cell boundary (returns 1 if it actually re-uploaded).
int  water_surface_mesh_update(water_surface_mesh *m, const water_wave_field *f,
                               float cam_x, float cam_z, float surface_y);

// draw it. assumes the surface shader is already bound + uniforms set.
void water_surface_mesh_draw(const water_surface_mesh *m);

#endif
