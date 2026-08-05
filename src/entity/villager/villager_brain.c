#include "villager_brain.h"
#include "villager_workstation.h"
#include "villager_def.h"
#include "../../world/block.h"
#include <math.h>
#include <stddef.h>
#define BRAIN_REPATH_INTERVAL 1.5f
#define BRAIN_PANIC_RANGE     6.0f
#define BRAIN_WORK_CYCLE      4.0f
#define BRAIN_WANDER_RANGE    5.0f
#define BRAIN_WANDER_REROLL   3.0f
static vec3 around(vec3 c, float r, rng *rng) {
    float ang = rng_frange(rng, 0.0f, 6.2831853f);
    float dist = rng_frange(rng, 1.0f, r);
    return vec3_new(c.x + cosf(ang) * dist, c.y, c.z + sinf(ang) * dist);
}

// decide the activity for this tick. schedule sets the baseline;
threats and
// damage override it. returns the chosen activity.
static villager_activity decide_activity(villager *v,
                                         const villager_brain_ctx *ctx) {
    // fresh damage or a nearby threat trumps everything.
    if (v->hurt_timer > 0.0f) return VILLAGER_ACT_PANIC;
    if (ctx->threat_active) {
        float d = vec3_distance(v->pos, ctx->threat_pos);
        if (d <= BRAIN_PANIC_RANGE) return VILLAGER_ACT_PANIC;
    }
    // babies never work; they wander or gather.
    villager_activity a = villager_schedule_at(&v->sched, ctx->day_t);
    if (v->is_baby && a == VILLAGER_ACT_WORK) a = VILLAGER_ACT_WANDER;
    if (v->prof == VILLAGER_PROF_UNEMPLOYED && a == VILLAGER_ACT_WORK)
        a = VILLAGER_ACT_WANDER;
    return a;
}

// SLEEP: walk to a claimed bed and idle there. claims one if needed.
static void do_sleep(villager *v, villager_nav *nav, villager_poi_set *pois,
                     world *w, float dt) {
    if (v->bed_poi < 0)
        villager_workstation_claim_bed(v, pois, v->pos);
villager_workstation_validate(v, pois, w);
return;
}
    if (!villager_workstation_validate(v, pois, w)) return;
vec3 ws = villager_poi_pos(pois, v->work_poi);
v->work_progress += dt;
int  have = 0;
int idx = villager_poi_nearest(pois, VILLAGER_POI_BELL, v->pos, v->id, 0.0f);
have = 1;
}
}

// WANDER: pick a random nearby spot and amble to it;
if (ctx->threat_active)
        away = vec3_sub(v->pos, ctx->threat_pos);
else
        away = vec3_new(cosf(v->yaw), 0, sinf(v->yaw));
away.y = 0;
float len = vec3_length(away);
if (len < 1e-3f) away = vec3_new(1, 0, 0);
else away = vec3_scale(away, 1.0f / len);
float speed = villager_def_get(v->prof)->walk_speed * 1.8f;
v->vel.x = away.x * speed;
v->vel.z = away.z * speed;
v->yaw = atan2f(away.x, away.z);
v->pos.x += v->vel.x * dt;
v->pos.z += v->vel.z * dt;
int bx = (int)floorf(v->pos.x), bz = (int)floorf(v->pos.z);
int by = (int)floorf(v->pos.y);
if (block_is_solid(world_get_block(w, bx, by - 1, bz)))
        v->pos.y = (float)by;
