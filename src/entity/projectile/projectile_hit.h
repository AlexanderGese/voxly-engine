#ifndef ENTITY_PROJECTILE_HIT_H
#define ENTITY_PROJECTILE_HIT_H
#include "projectile_types.h"
// entity hit detection. like the block sampler, we keep this decoupled from the
// mob registry: the caller hands us a flat array of (id, aabb) targets for the
// frame and we sweep the projectile segment against them. the world adapter
// (projectile_world) is what actually pulls those boxes off the mob list.
typedef struct {
    int  id;        // entity id; must be != owner to count (no self-shots)
    aabb box;       // world-space collision box this frame
} projectile_target;
typedef struct {
    int   id;       // which target got hit
    float t;        // param along the segment in [0,1] of the entry point
    vec3  point;    // contact point, world space
} projectile_entity_hit;
// sweep the segment from->to (a fat ray of the kind's radius) against every
// target, ignoring `owner_id` and any id flagged in `hit_mask_lo`. returns the
int projectile_hit_targets(projectile_kind kind, int owner_id,
                           uint32_t hit_mask_lo,
                           vec3 from, vec3 to,
                           const projectile_target *targets, int count,
                           projectile_entity_hit *out);
int projectile_hit_damage(projectile_kind kind, float speed, float speed0);
#endif
