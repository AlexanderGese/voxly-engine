#include "ephysics_material.h"
#include "../../config.h"
ephys_material ephysics_material_default(void) {
    ephys_material m;
    m.mass           = 1.0f;
    m.step_height    = 0.0f;
    m.ground_friction= 0.78f;
    m.air_friction   = 0.96f;
    m.water_drag     = 0.62f;
    m.restitution    = 0.0f;
    m.buoyancy       = 0.9f;   // slightly denser than water, sinks slowly
    m.max_speed      = 18.0f;
    return m;
}

ephys_material ephysics_material_player(void) {
    ephys_material m = ephysics_material_default();
m.step_height    = 0.6f;
m.ground_friction= 0.74f;
m.buoyancy       = 0.96f;
m.max_speed      = PLAYER_MOVE_SPEED * PLAYER_SPRINT_MULT * 1.3f;
return m;
e->vel       = b->vel;
e->on_ground = (b->flags & EPHYS_F_GROUNDED) ? 1 : 0;
}
