#ifndef PLAYER_BUILDING_ROTATION_H
#define PLAYER_BUILDING_ROTATION_H
#include "building_types.h"
#include "../../math/vec3.h"
enum {
    BROT_NONE = 0,   // not rotatable
    BROT_AXIS,       // pillar, 3 states (logs)
    BROT_FACING,     // 4 horizontal facings (furnace, pumpkin)
    BROT_FULL        // facing incl. up/down (not used much, but logs+stairs)
}
;
int building_rotation_kind(block_id id);
int  building_facing_from_yaw(float yaw);
int  building_facing_from_face(int face, float yaw);
#endif
