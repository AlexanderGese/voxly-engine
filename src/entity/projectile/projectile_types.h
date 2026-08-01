#ifndef ENTITY_PROJECTILE_TYPES_H
#define ENTITY_PROJECTILE_TYPES_H

#include <stdint.h>
#include "../../math/vec3.h"
#include "../../math/aabb.h"

// flying things: arrows, thrown rocks, snowballs, the odd spear. one struct
// for all of them tagged by kind, same trick the entity system pulls. we keep
// this header free of world/render/gl so the ballistics + collision code can be
// fuzzed against a flat block sampler without spinning up a real world.

// the kinds we model. tunables hang off these via projectile_kind_def().
typedef enum {
    PROJECTILE_NONE = 0,
    PROJECTILE_ARROW,       // sticks in blocks, pierces nothing, gravity-heavy
    PROJECTILE_BOLT,        // crossbow bolt: flatter arc, hits harder
    PROJECTILE_ROCK,        // thrown stone: bounces a little, never sticks
    PROJECTILE_SNOWBALL,    // splats on impact, no drop loot, light drag
    PROJECTILE_SPEAR,       // heavy, sticks deep, big damage, slow
    PROJECTILE_KIND_COUNT
} projectile_kind;

// where a projectile is in its little life. the world update is a state machine
// driven off this; render reads it to decide wobble vs frozen.
typedef enum {
    PROJ_STATE_FREE = 0,    // slot unused
    PROJ_STATE_FLYING,      // integrating ballistics, looking for a hit
    PROJ_STATE_STUCK,       // embedded in a block, decaying
    PROJ_STATE_SPENT        // hit something / expired, awaiting reap
} projectile_state;

// what a step's collision resolve decided happened this tick. the world loop
// fans out on this rather than peeking at internal fields.
typedef enum {
    PROJ_HIT_NONE = 0,      // clear air, keep flying
    PROJ_HIT_BLOCK,         // ran into a solid voxel
    PROJ_HIT_ENTITY,        // overlapped a mob aabb
    PROJ_HIT_FLUID,         // entered water/whatever, slows but no stop
    PROJ_HIT_VOID           // fell out the bottom of the world
} projectile_hitkind;

// static per-kind tunables. pure data, see projectile_types.c. a kind table is
// then just an array of these and nothing branches on the enum.
typedef struct {
    projectile_kind kind;
    const char *name;

    float mass;             // kg-ish, scales drag response only
    float radius;           // collision sphere radius, blocks
    float drag;             // linear air drag coefficient, 1/s
    float gravity_scale;    // multiplies config GRAVITY (snowball floats more)
    float restitution;      // 0=dead stop, 1=perfect bounce. arrows ~0.
    float speed;            // muzzle speed when fired, blocks/s

    int   sticks;           // embeds into the block it hits
    float stick_depth;      // how far past the face it buries, blocks
    float stick_decay;      // seconds it lingers stuck before despawn

    int   base_damage;      // hp on a flush entity hit at full speed
    float crit_speed;       // speed above which a 50% damage bonus applies
    float max_lifetime;     // seconds in flight before it gives up

    int   gravity_immune;   // true => flies dead straight (none do yet, but)
} projectile_def;

// the live thing. small and trivially copyable; pool stores these by value.
typedef struct {
    uint32_t id;            // unique, 0 == invalid
    projectile_kind kind;
    projectile_state state;

    vec3  pos;              // tip position, world space
    vec3  vel;              // blocks/s
    vec3  forward;          // unit heading, cached for render + tip math

    float age;              // seconds alive
    float spin;             // fletching roll angle, render only
    float speed0;           // muzzle speed, used to scale damage falloff

    int   owner_id;         // entity/player id that fired it; -1 == world
    int   stuck_bx, stuck_by, stuck_bz;  // block it's lodged in, if stuck
    vec3  stuck_off;        // offset from block origin to the embedded tip

    int   pierce_left;      // entities it can still pass through (spears>0)
    uint32_t hit_mask_lo;   // entity ids already damaged (cheap dedupe, low 32)
} projectile;

const projectile_def *projectile_kind_def(projectile_kind k);

// collision half-extents for a kind, as an aabb centered on the origin. used by
// the sweep test; arrows are basically a point but rocks have girth.
aabb projectile_kind_bounds(projectile_kind k);

#endif
