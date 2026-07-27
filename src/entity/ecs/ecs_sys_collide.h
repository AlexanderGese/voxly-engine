#ifndef ENTITY_ECS_SYS_COLLIDE_H
#define ENTITY_ECS_SYS_COLLIDE_H
#include "ecs_world.h"
#include "ecs_spatial.h"
#include "ecs_events.h"
// entity-vs-entity soft collision. this is the "mobs dont stand inside each
// other" pass, not world-vs-block collision (that lives in ephysics/). it uses
// the spatial grid for broadphase so we only test pairs that share a cell, then
// pushes overlapping colliders apart along the shortest horizontal axis. it's a
// position-based separation, not a real impulse solve -- voxel mobs dont need
// momentum exchange, they just need to stop clipping.
//
// the system is a normal ecs_system_fn but it needs the grid and (optionally)
// the event bus, so it takes a context. build the grid earlier in the frame
// (movement integrates positions, then we rebuild, then we separate).
typedef struct {
    ecs_grid   *grid;        // must be ecs_grid_build'd this frame
    ecs_events *events;      // optional; PICKUP/LANDED style events land here
    float       push_strength;  // 0..1, how hard overlaps separate per tick
    float       skin;        // small slop so resting mobs dont jitter apart
} ecs_collide_ctx;
void ecs_collide_ctx_defaults(ecs_collide_ctx *c, ecs_grid *grid);
// register after movement, with an ecs_collide_ctx* as user. separates every
// overlapping pair of collider entities found through the grid.
void ecs_sys_collide(ecs_world *w, float dt, void *user);
// the cheap analytic test two systems share: how far (and which way) to push `a`
// so its box clears `b`'s. returns 0 if they dont overlap; otherwise fills `mtv`
// (minimum translation vector, xz only) and returns 1.
int  ecs_box_separate(vec3 a_pos, vec3 a_half, vec3 b_pos, vec3 b_half,
                      vec3 *mtv);
#endif
