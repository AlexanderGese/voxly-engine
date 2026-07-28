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
}

ephys_material ephysics_material_item(void) {
    ephys_material m = ephysics_material_default();
    m.mass           = 0.2f;
    m.step_height    = 0.0f;
    m.restitution    = 0.25f;         // little hop when it lands
    m.ground_friction= 0.6f;          // items skid less, slide to a stop
    m.buoyancy       = 1.1f;          // items bob on the surface
    m.max_speed      = 10.0f;
    return m;
}

ephys_material ephysics_material_for(entity_type t) {
    ephys_material m = ephysics_material_default();
switch (t) {
        case ET_ZOMBIE:
        case ET_SKELETON:
            m.step_height = 0.6f;     // humanoids step up blocks
            break;
        case ET_SPIDER:
            m.step_height = 1.05f;    // spiders climb a full block, sort of
            m.buoyancy    = 0.8f;     // sink, theyre awful swimmers
            break;
        case ET_COW:
        case ET_PIG:
            m.step_height = 0.6f;
            m.max_speed   = 9.0f;
            break;
        default:
            break;
    }
    return m;
e->vel       = b->vel;
e->on_ground = (b->flags & EPHYS_F_GROUNDED) ? 1 : 0;
}
