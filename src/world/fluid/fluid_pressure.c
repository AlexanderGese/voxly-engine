#include "fluid_pressure.h"
#include "buoyancy.h"

// scale so numbers come out gameplay-friendly rather than physically real.
#define VOXL_FLUID_PRESSURE_SCALE  0.05f

int voxl_fluid_depth_above(const voxl_fluid_grid *g, int x, int y, int z) {
    const voxl_fluid_cell *here = voxl_fluid_at_const(g, x, y, z);
    if (!here || voxl_fluid_cell_empty(here)) return 0;
    int depth = 0;
    for (int yy = y + 1; yy < VOXL_FLUID_GRID_N; yy++) {
        const voxl_fluid_cell *c = voxl_fluid_at_const(g, x, yy, z);
        if (!c || voxl_fluid_cell_empty(c)) break;
        depth++;
    }
    return depth;
}

float voxl_fluid_pressure_at(const voxl_fluid_grid *g, int x, int y, int z,
                             float gravity) {
    const voxl_fluid_cell *c = voxl_fluid_at_const(g, x, y, z);
    if (!c || voxl_fluid_cell_empty(c)) return 0.0f;
    float rho = voxl_fluid_density_of(c);
    // depth includes the partial fill of this cell itself
    float depth = (float)voxl_fluid_depth_above(g, x, y, z);
    depth += (float)c->level / (float)VOXL_FLUID_FULL;
    return rho * gravity * depth * VOXL_FLUID_PRESSURE_SCALE;
}

void voxl_fluid_pressure_gradient(const voxl_fluid_grid *g, int x, int y, int z,
                                  float gravity, float *out_fx, float *out_fz) {
    float px0 = voxl_fluid_pressure_at(g, x - 1, y, z, gravity);
    float px1 = voxl_fluid_pressure_at(g, x + 1, y, z, gravity);
    float pz0 = voxl_fluid_pressure_at(g, x, y, z - 1, gravity);
    float pz1 = voxl_fluid_pressure_at(g, x, y, z + 1, gravity);
    // force points from high to low pressure => negative gradient
    if (out_fx) *out_fx = (px0 - px1) * 0.5f;
    if (out_fz) *out_fz = (pz0 - pz1) * 0.5f;
}
