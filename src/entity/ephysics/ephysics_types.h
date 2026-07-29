#ifndef ENTITY_EPHYSICS_TYPES_H
#define ENTITY_EPHYSICS_TYPES_H

#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include <stdint.h>

// shared types for the entity physics module. kept separate so the broadphase,
// sweep, and integrator headers dont have to include each other in a cycle.
// everything here is plain data, no world ptr, so it stays trivially testable.

// which axis a swept move actually touched. used by the resolver to zero out
// the right velocity component and to drive step-up.
typedef enum {
    EPHYS_AXIS_NONE = -1,
    EPHYS_AXIS_X    = 0,
    EPHYS_AXIS_Y    = 1,
    EPHYS_AXIS_Z    = 2
} ephys_axis;

// per-entity material/medium flags. a body can be in more than one at once
// (e.g. partially submerged AND touching a ladder), so these are bits.
enum {
    EPHYS_F_GROUNDED  = 1 << 0,   // standing on something solid this tick
    EPHYS_F_IN_WATER  = 1 << 1,   // aabb overlaps a fluid block
    EPHYS_F_SUBMERGED = 1 << 2,   // eye-level point is inside fluid
    EPHYS_F_ON_LADDER = 1 << 3,   // climbable block overlap
    EPHYS_F_STEPPED   = 1 << 4,   // we auto-stepped up a block this tick
    EPHYS_F_CEILING   = 1 << 5,   // bonked our head
    EPHYS_F_WALL_X    = 1 << 6,   // hit a wall along x
    EPHYS_F_WALL_Z    = 1 << 7    // hit a wall along z
};

// tunables per body. defaults come from ephysics_material_default(). pulled out
// of the body so mobs/players/items can share a const profile.
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

// the moving body. we keep half-extents instead of a full aabb so we can
// rebuild the box at an arbitrary position without carrying stale min/max.
typedef struct {
    vec3  pos;        // feet center (matches entity.pos convention)
    vec3  vel;        // m/s
    vec3  half;       // half-extents (x,z = width/2, y = height/2)
    float center_y;   // pos.y + center_y is the box center. usually half.y
    ephys_material mat;
    uint32_t flags;   // EPHYS_F_* from the last step
    float fluid_h;    // how deep we are in fluid, meters. 0 if dry
} ephys_body;

// result of a single swept move against the broadphase.
typedef struct {
    float       t;       // fraction of the move completed before contact (0..1)
    ephys_axis  axis;    // which axis the hit was on
    vec3        normal;  // contact normal, unit, points back at us
    int         hit;     // 1 if we touched anything
} ephys_hit;

#endif
