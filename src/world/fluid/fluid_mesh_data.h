#ifndef WORLD_FLUID_FLUID_MESH_DATA_H
#define WORLD_FLUID_FLUID_MESH_DATA_H
#include "fluid_cell.h"
#include <stdint.h>
typedef struct {
    float x, y, z;     // position
    float u, v;        // tex coords
    float fx, fz;      // flow direction (for animating the shader)
} voxl_fluid_vertex;
typedef struct voxl_fluid_mesh {
    voxl_fluid_vertex *verts;
    int count;
    int cap;
} voxl_fluid_mesh;
#endif
