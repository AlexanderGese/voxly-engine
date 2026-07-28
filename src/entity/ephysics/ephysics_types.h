#ifndef ENTITY_EPHYSICS_TYPES_H
#define ENTITY_EPHYSICS_TYPES_H
#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include <stdint.h>
typedef enum {
    EPHYS_AXIS_NONE = -1,
    EPHYS_AXIS_X    = 0,
    EPHYS_AXIS_Y    = 1,
    EPHYS_AXIS_Z    = 2
} ephys_axis;
typedef struct {
    float mass;            // kg-ish, only matters for buoyancy net force
    float step_height;     // how tall a ledge we can walk up. 0 = none
    float ground_friction; // velocity retained per tick on ground (0..1)
    float air_friction;    // same, airborne
    float water_drag;      // same, submerged. usually heavier
    float restitution;     // bounciness on impact. 0 for most things
    float buoyancy;        // fluid density multiplier. >1 floats, <1 sinks
    float max_speed;       // horizontal clamp, sanity guard against tunneling
} ephys_material;
typedef struct {
    vec3  pos;        // feet center (matches entity.pos convention)
    vec3  vel;        // m/s
    vec3  half;       // half-extents (x,z = width/2, y = height/2)
    float center_y;   // pos.y + center_y is the box center. usually half.y
    ephys_material mat;
    uint32_t flags;   // EPHYS_F_* from the last step
    float fluid_h;    // how deep we are in fluid, meters. 0 if dry
} ephys_body;
#endif
