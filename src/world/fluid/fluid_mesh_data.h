#ifndef WORLD_FLUID_FLUID_MESH_DATA_H
#define WORLD_FLUID_FLUID_MESH_DATA_H

#include "fluid_cell.h"
#include <stdint.h>

// builds flow vertex data into OUR OWN cpu-side buffer. absolutely no GL here
// the render layer can upload this however it wants. one quad per exposed
// fluid top face, with sloped corner heights so flowing fluid looks like it
// runs downhill.

typedef struct {
    float x, y, z;     // position
    float u, v;        // tex coords
    float fx, fz;      // flow direction (for animating the shader)
} voxl_fluid_vertex;

// grows on demand. caller owns it; call destroy to free.
typedef struct voxl_fluid_mesh {
    voxl_fluid_vertex *verts;
    int count;
    int cap;
} voxl_fluid_mesh;

void voxl_fluid_mesh_init(voxl_fluid_mesh *m);
void voxl_fluid_mesh_destroy(voxl_fluid_mesh *m);
void voxl_fluid_mesh_reset(voxl_fluid_mesh *m);   // keep capacity, count=0

// push one vertex. returns false on allocation failure.
bool voxl_fluid_mesh_push(voxl_fluid_mesh *m, voxl_fluid_vertex v);

// build top faces for every exposed fluid cell in the grid. a top face is
// "exposed" when the cell above is not a fluid/solid. returns vertex count.
int  voxl_fluid_mesh_build(voxl_fluid_mesh *m, const voxl_fluid_grid *g);

#endif
