#include "ai.h"
#include "../math/rng.h"
#include "../config.h"

#include <math.h>

static rng ai_rng;
static int rng_inited = 0;

static void init_rng(void) {
    if (!rng_inited) { rng_init(&ai_rng, 0xa1bada55); rng_inited = 1; }
}

static int is_hostile(entity_type t) {
    return t == ET_ZOMBIE || t == ET_SKELETON || t == ET_SPIDER;
}

static void face_towards(entity *e, vec3 target) {
    float dx = target.x - e->pos.x;
    float dz = target.z - e->pos.z;
    e->yaw = atan2f(dx, -dz);
}

static void walk_forward(entity *e, float speed, float dt) {
    e->pos.x += sinf(e->yaw) * speed * dt;
    e->pos.z -= cosf(e->yaw) * speed * dt;
}

void ai_tick(entity *e, const player *p, world *w, float dt) {
    init_rng();
    if (!e->alive) return;

    e->ai_timer += dt;

    if (e->hurt_timer > 0) {
        e->hurt_timer -= dt;
        e->ai_state = AI_HURT;
        if (e->hurt_timer <= 0) e->ai_state = AI_IDLE;
    }

    float dist = vec3_distance(e->pos, p->pos);

    switch (e->ai_state) {
    case AI_IDLE:
        if (e->ai_timer > 2.0f) {
            e->ai_timer = 0;
            e->yaw = rng_frange(&ai_rng, -3.14f, 3.14f);
            e->ai_state = AI_WANDER;
        }
        break;

    case AI_WANDER:
        walk_forward(e, 0.7f, dt);
        if (e->ai_timer > 3.0f) {
            e->ai_timer = 0;
            e->ai_state = AI_IDLE;
        }
        if (is_hostile(e->type) && dist < 10.0f) {
            e->ai_state = AI_CHASE;
            e->target_id = 1;   // just marker — player only target for now
        }
        break;

    case AI_CHASE:
        face_towards(e, p->pos);
        walk_forward(e, 1.8f, dt);
        if (dist > 20.0f) {
            e->ai_state = AI_WANDER;
        } else if (dist < 1.5f) {
            e->ai_state = AI_ATTACK;
            e->ai_timer = 0;
        }
        break;

    case AI_ATTACK:
        face_towards(e, p->pos);
        if (e->ai_timer > 0.8f) {
            e->ai_timer = 0;
            // the actual damage application happens in the game loop
        }
        if (dist > 2.5f) e->ai_state = AI_CHASE;
        break;

    case AI_HURT:
        // small push back
        e->pos.y += 0.05f * dt;
        break;

    case AI_FLEE: {
        vec3 away = vec3_sub(e->pos, p->pos);
        away = vec3_normalize(away);
        e->pos = vec3_add(e->pos, vec3_scale(away, 2.5f * dt));
        if (dist > 16.0f) e->ai_state = AI_WANDER;
        break;
    }
    }

    // rudimentary gravity (no world collision for entities yet)
    e->vel.y += GRAVITY * dt;
    e->pos.y += e->vel.y * dt;
    int by = (int)floorf(e->pos.y);
    int bx = (int)floorf(e->pos.x);
    int bz = (int)floorf(e->pos.z);
    if (by >= 0 && world_get_block(w, bx, by, bz)) {
        e->pos.y = (float)by + 1.0f;
        e->vel.y = 0;
        e->on_ground = 1;
    }
}
