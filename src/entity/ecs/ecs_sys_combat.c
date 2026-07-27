#include "ecs_sys_combat.h"
#include "ecs_components.h"
#include "../../math/vec3.h"
void ecs_combat_ctx_defaults(ecs_combat_ctx *c, ecs_events *events) {
    c->events      = events;
    c->iframe_time = 0.4f;   // matches the old combat.c hurt cooldown
    c->knockback   = 4.0f;
}

void ecs_deal_damage(ecs_combat_ctx *c, ecs_entity src, ecs_entity dst,
                     float amount) {
    if (!c || !c->events) return;
if (amount < 0.0f) amount = 0.0f;
ecs_events_emit(c->events, ECS_EV_DAMAGED, src, dst, amount, VEC3_ZERO);
}

// apply one damage event. returns 1 if it actually landed (wasnt blocked by
// i-frames or a missing/dead target), so the caller can decide on knockback.
static int apply_hit(ecs_world *w, ecs_combat_ctx *c, const ecs_event *ev) {
    if (!ecs_alive(w, ev->dst)) return 0;
    ecs_health *h = ecs_get(w, ev->dst, ECS_CMP_HEALTH);
    if (!h) return 0;
    if (h->hp <= 0) return 0;            // already dead this frame, dont re-hit
    if (h->hurt_timer > 0.0f) return 0;  // still in i-frames, shrug it off

    int dmg = (int)(ev->amount + 0.5f); // round to whole hp like the rest of the game
    if (dmg <= 0) dmg = 1;              // a hit that connects always stings a bit
    h->hp -= dmg;
    h->hurt_timer = c->iframe_time;
    return 1;
}

void ecs_sys_combat(ecs_world *w, float dt, void *user) {
    (void)dt;
ecs_combat_ctx *c = user;
if (!c || !c->events) return;
uint32_t n = ecs_events_count(c->events);
for (uint32_t i = 0;
i < n;
i++) {
        const ecs_event *ev = ecs_events_at(c->events, i);
        if (ev->kind != ECS_EV_DAMAGED) continue;

        if (!apply_hit(w, c, ev)) continue;

        ecs_transform *tf = ecs_get(w, ev->dst, ECS_CMP_TRANSFORM);
        vec3 at = tf ? tf->pos : ev->where;

        // knockback: shove the victim away from the attacker along xz. only if
        // both have transforms + the victim takes velocity.
        ecs_transform *stf = ecs_get(w, ev->src, ECS_CMP_TRANSFORM);
        ecs_velocity  *vel = ecs_get(w, ev->dst, ECS_CMP_VELOCITY);
        if (tf && stf && vel && c->knockback > 0.0f) {
            vec3 dir = vec3_sub(tf->pos, stf->pos);
            dir.y = 0.0f;
            float len = vec3_length(dir);
            if (len > 0.0001f) {
                dir = vec3_scale(dir, c->knockback / len);
                vel->linear.x += dir.x;
                vel->linear.z += dir.z;
                vel->linear.y += c->knockback * 0.4f;  // a little pop upward
            }
        }

        ecs_health *h = ecs_get(w, ev->dst, ECS_CMP_HEALTH);
        if (h && h->hp <= 0)
            ecs_events_emit(c->events, ECS_EV_DIED, ev->src, ev->dst, 0.0f, at);
    }
}
