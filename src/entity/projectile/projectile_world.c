#include "projectile_world.h"
#include "projectile_ballistic.h"
#include "projectile_collide.h"
#include "projectile_stick.h"
#include <math.h>
#include <stddef.h>
#define PROJ_MAX_STEP_DIST  0.45f
#define PROJ_MAX_SUBSTEPS   8
void projectile_world_init(projectile_world *pw, uint64_t seed) {
    projectile_pool_init(&pw->pool);
    projectile_rng_init(&pw->rng, seed);
    pw->sampler.block_at = NULL;
    pw->sampler.user = NULL;
    pw->on_damage = NULL;
    pw->damage_user = NULL;
    pw->void_y = -16.0f;        // bit below bedrock; nothing lives down there
    pw->sim_time = 0.0;
}

void projectile_world_set_sampler(projectile_world *pw, projectile_sampler s) {
    pw->sampler = s;
}

void projectile_world_set_damage_cb(projectile_world *pw,
                                    projectile_damage_cb cb, void *user) {
    pw->on_damage = cb;
    pw->damage_user = user;
}

uint32_t projectile_world_fire(projectile_world *pw, const projectile_shot *shot) {
    const projectile_def *d = projectile_kind_def(shot->kind);
uint32_t id = projectile_rng_next_id(&pw->rng);
projectile *p = projectile_pool_alloc(&pw->pool, id);
if (!p) {
        // pool full: try to make room by dropping the oldest free-flier, retry.
        if (!projectile_pool_cull_oldest(&pw->pool)) return 0;
        p = projectile_pool_alloc(&pw->pool, id);
        if (!p) return 0;
    }

    float speed = (shot->speed > 0.0f) ? shot->speed : d->speed;
vec3 dir = projectile_rng_cone(&pw->rng, shot->dir, shot->spread);
p->kind = shot->kind;
p->state = PROJ_STATE_FLYING;
p->pos = shot->origin;
p->forward = dir;
p->vel = vec3_add(vec3_scale(dir, speed),
                      projectile_rng_jitter(&pw->rng, speed * 0.01f));
p->speed0 = vec3_length(p->vel);
p->owner_id = shot->owner_id;
p->age = 0.0f;
p->spin = projectile_rng_frange(&pw->rng, 0.0f, 6.2831853f);
p->pierce_left = (shot->kind == PROJECTILE_SPEAR) ? 1 : 0;
p->hit_mask_lo = 0;
return id;
}

// mark an entity id as already-damaged in the dedupe mask (low ids only).
static void mark_hit(projectile *p, int id) {
    if (id >= 0 && id < 32) p->hit_mask_lo |= (1u << id);
}

// resolve one entity hit on a flying projectile. returns 1 if the projectile is
// spent (stop), 0 if it pierces and keeps going.
static int handle_entity_hit(projectile_world *pw, projectile *p,
                             const projectile_entity_hit *eh, float speed) {
    int dmg = projectile_hit_damage(p->kind, speed, p->speed0);
p->vel = VEC3_ZERO;
return 1;
pw->sim_time += dt;
for (int i = 0;
i < PROJECTILE_POOL_CAP;
}
