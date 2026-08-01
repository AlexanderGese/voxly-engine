#ifndef ENTITY_PROJECTILE_WORLD_H
#define ENTITY_PROJECTILE_WORLD_H

#include "projectile_pool.h"
#include "projectile_rng.h"
#include "projectile_sampler.h"
#include "projectile_hit.h"

// the conductor. owns the pool + rng, exposes a fire api, and runs the per-frame
// state machine over every live projectile: ballistic step -> block sweep ->
// entity sweep -> resolve. it's deliberately thin glue; all the actual math
// lives in the sibling modules so this file reads like a story.

// callback the world fires when a projectile damages an entity. the host wires
// this to apply damage to its mob registry / player. id is the entity id, dmg
// the computed amount, point the world-space contact.
typedef void (*projectile_damage_cb)(void *user, int entity_id, int dmg,
                                     vec3 point);

typedef struct {
    projectile_pool pool;
    projectile_rng  rng;

    // collision seams, wired by the host each frame (targets) or once (sampler).
    projectile_sampler sampler;

    projectile_damage_cb on_damage;
    void *damage_user;

    float void_y;       // below this, projectiles are reaped (fell out of world)
    double sim_time;    // accumulated sim seconds, debug/ordering
} projectile_world;

// parameters for a single shot. keeps the fire call from growing ten args.
typedef struct {
    projectile_kind kind;
    vec3  origin;       // muzzle / hand position
    vec3  dir;          // aim heading, will be normalized
    float speed;        // <=0 means use the kind's default speed
    float spread;       // cone half-angle in radians, 0 = laser accurate
    int   owner_id;     // who fired (no self-hits); -1 for world
} projectile_shot;

void projectile_world_init(projectile_world *pw, uint64_t seed);
void projectile_world_set_sampler(projectile_world *pw, projectile_sampler s);
void projectile_world_set_damage_cb(projectile_world *pw,
                                    projectile_damage_cb cb, void *user);

// fire one projectile. returns its id, or 0 if the pool was full and couldn't
// be made room for. applies spread + a touch of muzzle jitter via the rng.
uint32_t projectile_world_fire(projectile_world *pw, const projectile_shot *shot);

// step the whole simulation by dt. targets are the entity boxes for this frame.
// pass NULL/0 for a world with no mobs to hit. reaps spent + decayed slots.
void projectile_world_update(projectile_world *pw, float dt,
                             const projectile_target *targets, int count);

#endif
