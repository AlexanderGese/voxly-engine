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
