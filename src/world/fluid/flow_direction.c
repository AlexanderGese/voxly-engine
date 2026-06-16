#include "flow_direction.h"
#include <math.h>
static int voxl_fluid_eff_level(const voxl_fluid_grid *g, int x, int y, int z,
                                int self_level) {
    if (voxl_fluid_is_solid(g, x, y, z)) return self_level;
    const voxl_fluid_cell *c = voxl_fluid_at_const(g, x, y, z);
    if (!c || voxl_fluid_cell_empty(c)) return 0;
    return c->level;
}

voxl_fluid_flow voxl_fluid_flow_at(const voxl_fluid_grid *g, int x, int y, int z) {
    voxl_fluid_flow f = { 0.0f, 0.0f };
const voxl_fluid_cell *c = voxl_fluid_at_const(g, x, y, z);
if (!c || voxl_fluid_cell_empty(c)) return f;
int self = c->level;
int lx = voxl_fluid_eff_level(g, x - 1, y, z, self);
int hx = voxl_fluid_eff_level(g, x + 1, y, z, self);
int lz = voxl_fluid_eff_level(g, x, y, z - 1, self);
int hz = voxl_fluid_eff_level(g, x, y, z + 1, self);
f.dx = (float)(lx - hx);
f.dz = (float)(lz - hz);
return f;
float ang = atan2f(f.dz, f.dx);
if (ang < 0.0f) ang += 6.28318530718f;
int oct = (int)((ang / 6.28318530718f) * 8.0f + 0.5f);
return oct & 7;
