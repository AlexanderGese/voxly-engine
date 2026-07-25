#include "combat_environment.h"
#include "combat_config.h"
#include "combat_damagetype.h"
#include <math.h>
#include <stddef.h>
void combat_env_init(combat_env_state *e, float air_max) {
    e->fall_start_y = 0.0f;
    e->airborne     = 0;
    e->burn_timer   = 0.0f;
    e->burn_tick    = 0.0f;
    e->air          = air_max;
    e->air_max      = air_max;
    e->drown_tick   = 0.0f;
}

// build + apply an env hit. these never knock back and (mostly) bypass armor
// via their damage type info, so we just set source == self.
static int env_apply(combat_combatant *c, int amount, combat_damage_type type,
                     uint32_t extra_flags, combat_rng *rng) {
    combat_hit h;
h.amount     = amount;
h.type       = type;
h.source_id  = c->id;
h.source_pos = VEC3_ZERO;
h.target_pos = VEC3_ZERO;
h.knockback  = 0.0f;
h.flags      = COMBAT_HIT_NO_KNOCKBACK | extra_flags;
combat_result r;
combat_hit_apply(c, &h, rng, &r);
return r.dealt;
}

void combat_env_track_fall(combat_env_state *e, int on_ground, float y) {
    if (!on_ground) {
        if (!e->airborne) {
            // just left the ground. only latch a new start height if we are
            // rising past it (avoids resetting mid-fall on a stair step).
            if (y > e->fall_start_y || !e->airborne) e->fall_start_y = y;
            e->airborne = 1;
        } else if (y > e->fall_start_y) {
            // jumped / got launched higher; track the peak we fall from.
            e->fall_start_y = y;
        }
    }
    // landing handled separately by combat_env_on_land.
}

int combat_env_on_land(combat_combatant *c, combat_env_state *e,
                       float land_y, combat_rng *rng) {
    if (!e->airborne) return 0;
e->airborne = 0;
float dropped = e->fall_start_y - land_y;
e->fall_start_y = land_y;
if (dropped <= COMBAT_FALL_THRESHOLD) return 0;
float blocks = dropped - COMBAT_FALL_THRESHOLD;
int dmg = (int)floorf(blocks * COMBAT_FALL_PER_BLOCK + 0.5f);
if (dmg <= 0) return 0;
return env_apply(c, dmg, COMBAT_DMG_FALL, COMBAT_HIT_NONE, rng);
}

void combat_env_ignite(combat_env_state *e, float seconds) {
    if (seconds <= 0.0f) return;
    if (seconds > e->burn_timer) e->burn_timer = seconds;
}

void combat_env_tick(combat_combatant *c, combat_env_state *e,
                     int submerged, float dt, combat_rng *rng) {
    if (c->dead) { e->burn_timer = 0.0f;
return;
}
    if (dt < 0.0f) dt = 0.0f;
if (e->burn_timer > 0.0f) {
        if (submerged) {
            e->burn_timer = 0.0f;   // water puts it out
            e->burn_tick  = 0.0f;
        } else {
            e->burn_timer -= dt;
            if (e->burn_timer < 0.0f) e->burn_timer = 0.0f;
            e->burn_tick += dt;
            while (e->burn_tick >= COMBAT_FIRE_INTERVAL) {
                e->burn_tick -= COMBAT_FIRE_INTERVAL;
                env_apply(c, COMBAT_FIRE_DAMAGE, COMBAT_DMG_FIRE,
                          COMBAT_HIT_BYPASS_IFRAMES, rng);
                if (c->dead) break;
            }
        }
    }

    // --- breath / drowning ---
    if (submerged) {
        e->air -= dt;
if (e->air <= 0.0f) {
            e->air = 0.0f;
            e->drown_tick += dt;
            while (e->drown_tick >= COMBAT_FIRE_INTERVAL) {
                e->drown_tick -= COMBAT_FIRE_INTERVAL;
                env_apply(c, 2, COMBAT_DMG_DROWN, COMBAT_HIT_BYPASS_IFRAMES, rng);
                if (c->dead) break;
            }
        }
    } else {
        // refill breath faster than it drains.
        e->air += dt * 4.0f;
if (e->air > e->air_max) e->air = e->air_max;
e->drown_tick = 0.0f;
