#include "villager.h"
#include "villager_def.h"

#include <stddef.h>

// villager body dimensions. a touch slimmer and shorter than the player so
// they fit through their own doorways.
#define VILLAGER_WIDTH   0.6f
#define VILLAGER_HEIGHT  1.95f
#define VILLAGER_BABY_HEIGHT 0.95f

// how long a baby takes to grow up, in seconds. ~20 minutes, one day.
#define VILLAGER_GROW_SECONDS 1200.0f

#define VILLAGER_BASE_HP 20

static void common_init(villager *v, uint32_t id, vec3 pos, uint64_t seed) {
    v->id = id;
    v->pos = pos;
    v->vel = vec3_new(0, 0, 0);
    v->yaw = 0.0f;
    v->on_ground = 0;
    v->hp = VILLAGER_BASE_HP;
    v->max_hp = VILLAGER_BASE_HP;
    v->hurt_timer = 0.0f;
    v->activity = VILLAGER_ACT_WANDER;
    v->act_timer = 0.0f;
    v->repath_timer = 0.0f;
    v->grow_timer = 0.0f;
    v->work_progress = 0.0f;
    v->wander_dir = 0.0f;
    v->bed_poi = -1;
    v->work_poi = -1;
    rng_init(&v->rng, seed ? seed : (id * 2654435761u + 1u));
    villager_gossip_init(&v->gossip);
}

villager villager_make(uint32_t id, villager_profession prof, vec3 pos, uint64_t seed) {
    villager v;
    common_init(&v, id, pos, seed);
    v.is_baby = 0;
    villager_set_profession(&v, prof);
    return v;
}

villager villager_make_baby(uint32_t id, vec3 pos, uint64_t seed) {
    villager v;
    common_init(&v, id, pos, seed);
    v.is_baby = 1;
    v.grow_timer = VILLAGER_GROW_SECONDS;
    // babies are unemployed until they grow and find work.
    villager_set_profession(&v, VILLAGER_PROF_UNEMPLOYED);
    return v;
}

void villager_set_profession(villager *v, villager_profession prof) {
    // a nitwit is a nitwit for life. so it goes.
    if (v->prof == VILLAGER_PROF_NITWIT) return;
    if (prof < 0 || prof >= VILLAGER_PROF_COUNT) prof = VILLAGER_PROF_UNEMPLOYED;

    v->prof = prof;
    v->level = 1;
    villager_schedule_build(&v->sched, prof);
    villager_trades_init(&v->trades, prof);
    v->work_progress = 0.0f;
}

aabb villager_aabb(const villager *v) {
    float h = v->is_baby ? VILLAGER_BABY_HEIGHT : VILLAGER_HEIGHT;
    float hw = VILLAGER_WIDTH * 0.5f;
    vec3 min = vec3_new(v->pos.x - hw, v->pos.y,       v->pos.z - hw);
    vec3 max = vec3_new(v->pos.x + hw, v->pos.y + h,   v->pos.z + hw);
    return aabb_make(min, max);
}

int villager_is_alive(const villager *v) {
    return v->id != 0 && v->hp > 0;
}

int villager_hurt(villager *v, int amount) {
    if (amount <= 0) return 0;
    v->hp -= amount;
    v->hurt_timer = 0.5f;
    // getting hit nukes whatever they were doing; the brain reads this.
    if (v->hp <= 0) {
        v->hp = 0;
        return 1;
    }
    return 0;
}
