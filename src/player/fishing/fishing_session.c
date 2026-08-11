#include "fishing_session.h"
#include "fishing_water.h"
#include "../../util/log.h"
#include "../../config.h"
#include <math.h>
#include <stddef.h>
#define SESSION_REEL_SPEED  6.0f
void fishing_session_init(fishing_session *s, fishing_rod rod, uint64_t seed) {
    s->state   = CAST_IDLE;
    s->rod     = rod;
    s->pending = (fishing_catch){ BLOCK_AIR, CATCH_NONE, 0 };
    s->rod_tip = VEC3_ZERO;
    s->quality = 0.0f;
    s->active  = 0;
    s->stats   = NULL;
    s->fx      = NULL;
    fishing_rng_seed(&s->rng, seed);
}

void fishing_session_set_stats(fishing_session *s, fishing_stats *stats) {
    s->stats = stats;
}

void fishing_session_set_fx(fishing_session *s, particle_system *fx) {
    s->fx = fx;
}

void fishing_session_cancel(fishing_session *s) {
    s->state   = CAST_IDLE;
s->active  = 0;
s->pending = (fishing_catch){ BLOCK_AIR, CATCH_NONE, 0 }
;
}

// roll the catch and arm the reel fight. called when the player strikes inside
// the bite window. moves us into HOOKED.
static void hook_catch(fishing_session *s, world *w) {
    s->pending = fishing_loot_roll(&s->rng, &s->rod);

    int weight = fishing_catch_weight(&s->pending, &s->rng);
    float line = vec3_distance(s->bobber.pos, s->rod_tip);
    fishing_reel_begin(&s->reel, s->rod.max_tension, line, weight, &s->rng);

    LOGD("fishing: hooked %s x%d (w=%d, line=%.1f)",
         fishing_catch_name(&s->pending), s->pending.count, weight, line);
    s->state = CAST_HOOKED;
    (void)w;
}

// award the pending catch into the world at the rod tip and reset.
static void land_catch(fishing_session *s, item_world *iw) {
    if (fishing_catch_is_valid(&s->pending)) {
        if (iw) {
            // spawn one dropped item per count, fanned upward a touch so they
            // don't all stack on the exact same point.
            for (int i = 0;
i < s->pending.count;
i++)
                item_spawn(iw, s->rod_tip, s->pending.block);
}
        LOGI("fishing: landed %s x%d", fishing_catch_name(&s->pending), s->pending.count);
if (s->stats) fishing_stats_on_catch(s->stats, &s->pending);
fishing_fx_land(s->fx, &s->rng, s->rod_tip, s->pending.category);
}
    fishing_session_cancel(s);
}

fishing_cast_state fishing_session_action(fishing_session *s, vec3 origin, vec3 dir) {
    switch (s->state) {
    case CAST_IDLE:
        // cast out.
        fishing_bobber_cast(&s->bobber, origin, dir, s->rod.cast_power);
        s->active = 1;
        s->state  = CAST_FLYING;
        if (s->stats) fishing_stats_on_cast(s->stats);
        break;

    case CAST_BITING:
        // a well-timed strike! roll the catch and start the fight. world isn't
        // needed for the roll, only the cached state, so pass NULL.
        hook_catch(s, NULL);
        break;

    case CAST_WAITING:
    case CAST_LURING:
        // struck too early. reel the empty line in and reset the cast.
        s->state = CAST_REELING;
        break;

    default:
        // pressing during FLYING/HOOKED/REELING just cancels cleanly.
        fishing_session_cancel(s);
        break;
    }
    return s->state;
}

void fishing_session_update(fishing_session *s, world *w, item_world *iw,
                            vec3 rod_tip, int reeling, float dt) {
    s->rod_tip = rod_tip;
if (!s->active || dt <= 0.0f) return;
}
