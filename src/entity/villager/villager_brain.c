#include "villager_brain.h"
#include "villager_workstation.h"
#include "villager_def.h"
#include "../../world/block.h"

#include <math.h>
#include <stddef.h>

// how often a villager will recompute a path while pursuing the same goal.
#define BRAIN_REPATH_INTERVAL 1.5f
// distance at which a threat flips us into panic.
#define BRAIN_PANIC_RANGE     6.0f
// seconds of work to bank one restock cycle.
#define BRAIN_WORK_CYCLE      4.0f
// wander step length and how often we re-roll a direction.
#define BRAIN_WANDER_RANGE    5.0f
#define BRAIN_WANDER_REROLL   3.0f

static vec3 around(vec3 c, float r, rng *rng) {
    float ang = rng_frange(rng, 0.0f, 6.2831853f);
    float dist = rng_frange(rng, 1.0f, r);
    return vec3_new(c.x + cosf(ang) * dist, c.y, c.z + sinf(ang) * dist);
}

// decide the activity for this tick. schedule sets the baseline; threats and
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

    if (v->bed_poi >= 0) {
        vec3 bed = villager_poi_pos(pois, v->bed_poi);
        if (vec3_distance(v->pos, bed) > 1.2f) {
            villager_nav_set_goal(nav, w, v->pos, bed);
            villager_nav_advance(nav, v, w, dt);
        } else {
            // tucked in. stop moving and heal a hair overnight.
            v->vel.x = v->vel.z = 0.0f;
            if (v->hp < v->max_hp && v->act_timer > 4.0f) {
                v->hp++;
                v->act_timer = 0.0f;
            }
        }
    }
}

// WORK: validate the station, walk to it, bank work-progress into restocks.
static void do_work(villager *v, villager_nav *nav, villager_poi_set *pois,
                    world *w, float dt) {
    if (v->work_poi < 0) {
        villager_workstation_seek_job(v, pois, v->pos);
        villager_workstation_validate(v, pois, w);
        return;
    }
    if (!villager_workstation_validate(v, pois, w)) return;

    vec3 ws = villager_poi_pos(pois, v->work_poi);
    if (vec3_distance(v->pos, ws) > 1.6f) {
        villager_nav_set_goal(nav, w, v->pos, ws);
        villager_nav_advance(nav, v, w, dt);
        return;
    }

    // at the station: face it, grind out work, restock on each cycle.
    v->vel.x = v->vel.z = 0.0f;
    v->work_progress += dt;
    if (v->work_progress >= BRAIN_WORK_CYCLE) {
        v->work_progress -= BRAIN_WORK_CYCLE;
        int amt = villager_def_get(v->prof)->restock_amount;
        villager_trade_restock(&v->trades, amt);
        v->level = v->trades.level;
    }
}

// GATHER: head to the bell / village center and mingle.
static void do_gather(villager *v, villager_nav *nav, villager_poi_set *pois,
                      world *w, const villager_brain_ctx *ctx, float dt) {
    vec3 center;
    int  have = 0;

    int idx = villager_poi_nearest(pois, VILLAGER_POI_BELL, v->pos, v->id, 0.0f);
    if (idx >= 0) { center = villager_poi_pos(pois, idx); have = 1; }
    else if (ctx->have_bell) { center = ctx->bell_pos; have = 1; }

    if (have) {
        if (vec3_distance(v->pos, center) > 2.5f) {
            villager_nav_set_goal(nav, w, v->pos, center);
            villager_nav_advance(nav, v, w, dt);
        } else {
            v->vel.x = v->vel.z = 0.0f;   // standing around, gossiping
        }
    } else {
        // no bell known; degrade to a gentle wander.
        if (!villager_nav_has_goal(nav)) {
            vec3 g = around(v->pos, BRAIN_WANDER_RANGE, &v->rng);
            villager_nav_set_goal(nav, w, v->pos, g);
        }
        villager_nav_advance(nav, v, w, dt);
    }
}

// WANDER: pick a random nearby spot and amble to it; re-roll on arrival.
static void do_wander(villager *v, villager_nav *nav, world *w, float dt) {
    v->wander_dir += dt;
    if (!villager_nav_has_goal(nav) || v->wander_dir > BRAIN_WANDER_REROLL) {
        v->wander_dir = 0.0f;
        vec3 g = around(v->pos, BRAIN_WANDER_RANGE, &v->rng);
        villager_nav_set_goal(nav, w, v->pos, g);
    }
    villager_nav_advance(nav, v, w, dt);
}

// PANIC: run directly away from the threat at boosted speed, ignore pathing.
static void do_panic(villager *v, const villager_brain_ctx *ctx, world *w, float dt) {
    vec3 away;
    if (ctx->threat_active)
        away = vec3_sub(v->pos, ctx->threat_pos);
    else
        away = vec3_new(cosf(v->yaw), 0, sinf(v->yaw));   // hurt by unknown
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

    // crude ground snap so a panicking villager doesn't moonwalk off cliffs.
    int bx = (int)floorf(v->pos.x), bz = (int)floorf(v->pos.z);
    int by = (int)floorf(v->pos.y);
    if (block_is_solid(world_get_block(w, bx, by - 1, bz)))
        v->pos.y = (float)by;
}

void villager_brain_tick(villager *v, villager_nav *nav,
                         villager_poi_set *pois, world *w,
                         const villager_brain_ctx *ctx, float dt) {
    if (!villager_is_alive(v)) return;

    // timers
    if (v->hurt_timer > 0.0f) v->hurt_timer -= dt;
    if (v->hurt_timer < 0.0f) v->hurt_timer = 0.0f;
    villager_gossip_tick(&v->gossip, dt);

    // babies grow up and then look for work.
    if (v->is_baby) {
        v->grow_timer -= dt;
        if (v->grow_timer <= 0.0f) {
            v->is_baby = 0;
            // stays unemployed until it claims a station.
        }
    }

    villager_activity want = decide_activity(v, ctx);
    if (want != v->activity) {
        v->activity = want;
        v->act_timer = 0.0f;
        villager_nav_reset(nav);   // new goal context, drop stale path
    }
    v->act_timer += dt;

    // periodically nudge unemployed adults to seek a job in the background.
    v->repath_timer -= dt;
    if (v->repath_timer <= 0.0f) {
        v->repath_timer = BRAIN_REPATH_INTERVAL;
        if (!v->is_baby && v->prof == VILLAGER_PROF_UNEMPLOYED)
            villager_workstation_seek_job(v, pois, v->pos);
    }

    switch (v->activity) {
    case VILLAGER_ACT_SLEEP:  do_sleep(v, nav, pois, w, dt);          break;
    case VILLAGER_ACT_WORK:   do_work(v, nav, pois, w, dt);           break;
    case VILLAGER_ACT_GATHER: do_gather(v, nav, pois, w, ctx, dt);    break;
    case VILLAGER_ACT_WANDER: do_wander(v, nav, w, dt);               break;
    case VILLAGER_ACT_PANIC:  do_panic(v, ctx, w, dt);               break;
    default: break;
    }
}
