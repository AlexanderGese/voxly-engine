#include "xp_collect.h"

#include <math.h>
#include <stddef.h>

#include "xp_config.h"
#include "xp_orb.h"
#include "xp_tier.h"

void xp_collect_init(xp_collect *c) {
    c->orbs_absorbed = 0;
    c->xp_gained = 0;
    c->pickup_cooldown = 0.0f;
}

int xp_collect_run(xp_collect *c, xp_orb_pool *pool, vec3 feet,
                   xp_state *state, xp_event_log *log, float dt) {
    c->orbs_absorbed = 0;
    c->xp_gained = 0;
    if (c->pickup_cooldown > 0.0f) c->pickup_cooldown -= dt;

    // player capsule center is a bit above the feet; pick orbs against a
    // squared range so we skip the sqrt in the hot loop.
    vec3 center = vec3_new(feet.x, feet.y + 1.0f, feet.z);
    float pr = XP_ORB_PICKUP_RANGE;
    float pr2 = pr * pr;

    int cap = xp_orb_capacity(pool);
    int total = 0;

    for (int i = 0; i < cap; i++) {
        xp_orb *o = xp_orb_at(pool, i);
        if (!o || !o->alive) continue;

        vec3 d = vec3_sub(o->pos, center);
        if (vec3_length_sq(d) > pr2) continue;

        // absorb. fresh orbs (age < 0.4) refuse pickup so they pop outward
        // first; otherwise you'd vacuum them the instant they spawn on you.
        if (o->age < 0.4f) continue;

        int value = xp_tier_get(o->tier)->value;
        total += value;
        c->orbs_absorbed++;

        if (log)
            xp_event_push(log, XP_EV_GAIN, value, o->pos);

        xp_orb_kill(pool, i);
    }

    if (total > 0) {
        xp_state_add(state, total);
        c->xp_gained = total;
        if (c->pickup_cooldown <= 0.0f)
            c->pickup_cooldown = 0.06f;
    }
    return total;
}

int xp_collect_merge(xp_orb_pool *pool) {
    int cap = xp_orb_capacity(pool);
    float mr = XP_ORB_MERGE_RANGE;
    float mr2 = mr * mr;
    int merges = 0;

    // O(n^2) over live orbs. n is small (a burst is <= ~64) and this runs at
    // a low cadence from the facade, so the quadratic is fine.
    for (int i = 0; i < cap; i++) {
        xp_orb *a = xp_orb_at(pool, i);
        if (!a || !a->alive) continue;
        if (a->magnetized) continue; // don't disturb orbs already homing in
        if (a->tier >= XP_ORB_TIER_COUNT - 1) continue; // can't promote top tier

        for (int j = i + 1; j < cap; j++) {
            xp_orb *b = xp_orb_at(pool, j);
            if (!b || !b->alive) continue;
            if (b->tier != a->tier || b->magnetized) continue;

            vec3 d = vec3_sub(a->pos, b->pos);
            if (vec3_length_sq(d) > mr2) continue;

            // promote a into the next tier, consume b. value isn't perfectly
            // conserved (next tier is worth a bit more) but it's close and it
            // keeps the field tidy. think of it as a coalescence bonus.
            a->tier += 1;
            a->pos = vec3_lerp(a->pos, b->pos, 0.5f);
            a->vel = vec3_lerp(a->vel, b->vel, 0.5f);
            xp_orb_kill(pool, j);
            merges++;
            break; // a changed tier; bail this inner loop and move on
        }
    }
    return merges;
}
