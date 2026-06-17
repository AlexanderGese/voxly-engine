#include "viscosity.h"

voxl_fluid_viscosity voxl_fluid_viscosity_for(uint8_t kind) {
    voxl_fluid_viscosity v;
    switch (kind) {
    case VOXL_FLUID_LAVA:
        v.drag = 6.0f;        // really gloopy, slows you to a crawl
        v.flow_delay = 4;
        v.slope_bias = 0.9f;
        break;
    case VOXL_FLUID_WATER:
        v.drag = 2.5f;
        v.flow_delay = 1;
        v.slope_bias = 0.5f;
        break;
    default:                  // air / unknown, basically no drag
        v.drag = 0.0f;
        v.flow_delay = 1;
        v.slope_bias = 0.0f;
        break;
    }
    return v;
}

float voxl_fluid_apply_drag(const voxl_fluid_viscosity *v, float vel, float dt) {
    if (!v) return vel;
    float k = 1.0f - v->drag * dt;
    if (k < 0.0f) k = 0.0f;   // dont overshoot into negative on big dt
    return vel * k;
}

bool voxl_fluid_should_flow(const voxl_fluid_viscosity *v, int tick) {
    if (!v || v->flow_delay <= 1) return true;
    return (tick % v->flow_delay) == 0;
}

voxl_fluid_viscosity voxl_fluid_viscosity_lerp(voxl_fluid_viscosity a,
                                               voxl_fluid_viscosity b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    voxl_fluid_viscosity r;
    r.drag = a.drag + (b.drag - a.drag) * t;
    r.slope_bias = a.slope_bias + (b.slope_bias - a.slope_bias) * t;
    // flow_delay is discrete, just snap to whichever is closer
    r.flow_delay = (t < 0.5f) ? a.flow_delay : b.flow_delay;
    return r;
}
