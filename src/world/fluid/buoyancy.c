#include "buoyancy.h"

#include <math.h>

// drag while submerged. higher = sinks/floats less twitchy.
#define VOXL_FLUID_DRAG  3.5f

float voxl_fluid_density_of(const voxl_fluid_cell *c) {
    if (!c || voxl_fluid_cell_empty(c)) return 0.0f;
    if (c->kind == VOXL_FLUID_LAVA) return VOXL_FLUID_LAVA_DENSITY;
    if (c->kind == VOXL_FLUID_WATER) return VOXL_FLUID_WATER_DENSITY;
    return 0.0f;
}

float voxl_fluid_submerged_fraction(float body_bottom, float body_height,
                                    float surface_y) {
    if (body_height <= 0.0f) return 0.0f;
    float body_top = body_bottom + body_height;
    if (surface_y <= body_bottom) return 0.0f;   // fully above water
    if (surface_y >= body_top)    return 1.0f;   // fully under
    return (surface_y - body_bottom) / body_height;
}

float voxl_fluid_buoyant_accel(const voxl_fluid_body *b, float fluid_density,
                               float submerged, float gravity) {
    if (!b) return 0.0f;
    if (submerged < 0.0f) submerged = 0.0f;
    if (submerged > 1.0f) submerged = 1.0f;
    // weight pulls down, displaced fluid pushes up. relative to body density.
    // accel = g * (rho_fluid * submerged / rho_body - 1)
    float rho_body = b->density > 0.001f ? b->density : 0.001f;
    float up = gravity * (fluid_density * submerged / rho_body);
    return up - gravity;
}

float voxl_fluid_body_integrate(voxl_fluid_body *b, float fluid_density,
                                float submerged, float gravity, float dt) {
    if (!b) return 0.0f;
    float a = voxl_fluid_buoyant_accel(b, fluid_density, submerged, gravity);
    b->vy += a * dt;
    // viscous drag proportional to how submerged we are
    float drag = 1.0f - VOXL_FLUID_DRAG * submerged * dt;
    if (drag < 0.0f) drag = 0.0f;
    b->vy *= drag;
    // clamp tiny velocities so floating objects actually come to rest
    if (fabsf(b->vy) < 0.0005f) b->vy = 0.0f;
    return b->vy;
}
