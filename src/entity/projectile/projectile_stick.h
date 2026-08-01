#ifndef ENTITY_PROJECTILE_STICK_H
#define ENTITY_PROJECTILE_STICK_H

#include "projectile_types.h"
#include "projectile_collide.h"

// what to do when a projectile meets a block. arrows/spears embed (stick),
// rocks bounce if they're moving fast enough into the face, everything else
// just stops dead. this module owns that little decision and the resulting
// state transition so the world loop stays a flat switch.

typedef enum {
    PROJ_RESOLVE_STICK = 0, // froze, embedded in the block
    PROJ_RESOLVE_BOUNCE,    // reflected off the face, still flying
    PROJ_RESOLVE_STOP       // dead stop, mark spent (no stick visual)
} projectile_resolve;

// resolve a block hit. mutates p: on STICK it sets state/stuck fields, on BOUNCE
// it reflects vel and nudges pos off the surface, on STOP it marks spent. the
// hit must come from projectile_collide_segment with kind == PROJ_HIT_BLOCK.
projectile_resolve projectile_stick_resolve(projectile *p,
                                            const projectile_block_hit *hit);

// the world-space tip position of a stuck projectile, for render. recomputed
// from the block origin + stored embed offset so it survives chunk reloads.
vec3 projectile_stick_tip(const projectile *p);

// advance a stuck projectile's decay timer. returns 1 when it should despawn.
int projectile_stick_decay(projectile *p, float dt);

#endif
