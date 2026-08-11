#include "fishing_session.h"
#include "fishing_water.h"
#include "../../util/log.h"
#include "../../config.h"
#include <math.h>
#include <stddef.h>
// how fast the bobber gets dragged back during a reel-in (the REELING phase,
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
switch (s->state) {
    case CAST_FLYING:
        fishing_bobber_update(&s->bobber, w, dt);
        if (s->bobber.landed) {
            if (s->bobber.water == WATER_NONE) {
                // landed on land. nothing bites; just sit until the player reels.
                s->state = CAST_WAITING;
                s->quality = 0.0f;
                // begin a wait it can never really pay off — quality 0.
                fishing_bite_begin(&s->bite, &s->rng, &s->rod, 0.0f);
            } else {
                int wx = (int)floorf(s->bobber.pos.x);
                int wz = (int)floorf(s->bobber.pos.z);
                int found = 0;
                int sy = fishing_water_surface_y(w, wx, wz, &found);
                s->quality = found ? fishing_water_quality(w, wx, sy, wz) : 0.1f;
                fishing_bite_begin(&s->bite, &s->rng, &s->rod, s->quality);
                fishing_fx_splash(s->fx, &s->rng, s->bobber.pos);
                s->state = CAST_WAITING;
            }
        }
        break;

    case CAST_WAITING:
        fishing_bobber_update(&s->bobber, w, dt);
        fishing_bobber_set_dunk(&s->bobber, 0.0f, dt);
        if (s->bobber.water != WATER_NONE && fishing_bite_tick_wait(&s->bite, dt))
            s->state = CAST_LURING;
        break;

    case CAST_LURING:
        fishing_bobber_update(&s->bobber, w, dt);
        // ease the bobber down as the fish approaches.
        {
            float prog = fishing_bite_lure_progress(&s->bite);
            fishing_bobber_set_dunk(&s->bobber, prog * 0.5f, dt);
            // sparse ripples, denser as the bite nears. roll so we don't emit
            // every single frame and drown the particle pool.
            if (fishing_rng_chance(&s->rng, dt * (2.0f + prog * 6.0f)))
                fishing_fx_lure_ripple(s->fx, &s->rng, s->bobber.pos, prog);
        }
        if (fishing_bite_tick_lure(&s->bite, dt)) {
            fishing_fx_bite(s->fx, &s->rng, s->bobber.pos);
            s->state = CAST_BITING;
        }
        break;

    case CAST_BITING:
        fishing_bobber_update(&s->bobber, w, dt);
        // full dunk: the tell that you should strike now.
        fishing_bobber_set_dunk(&s->bobber, 1.0f, dt);
        if (fishing_bite_tick_window(&s->bite, dt)) {
            // missed the window. shrug, re-roll a fresh wait.
            LOGD("fishing: missed the bite (#%d)", s->bite.misses);
            if (s->stats) fishing_stats_on_miss(s->stats);
            fishing_bite_begin(&s->bite, &s->rng, &s->rod, s->quality);
            s->state = CAST_WAITING;
        }
        break;

    case CAST_HOOKED: {
        // the active fight. tug-of-war between the catch and the reel button.
        fishing_reel_step_fight(&s->reel, &s->rng, dt);
        fishing_reel_result rr = fishing_reel_apply(&s->reel, reeling, dt);
        // drag the bobber in proportion to how much line is left.
        vec3 d = vec3_sub(s->rod_tip, s->bobber.pos);
        float dl = vec3_length(d);
        if (dl > 0.001f) {
            float want = s->reel.line;            // target distance == line out
            float move = (dl - want);
            if (move > 0.0f)
                s->bobber.pos = vec3_add(s->bobber.pos,
                                         vec3_scale(d, (move / dl)));
        }
        if (rr == REEL_LANDED) {
            s->state = CAST_REELING;              // final short pull-in, then award
        } else if (rr == REEL_SNAPPED) {
            LOGI("fishing: line snapped, lost the %s", fishing_catch_name(&s->pending));
            if (s->stats) fishing_stats_on_snap(s->stats);
            s->state = CAST_SNAPPED;
        }
        break;
    }

    case CAST_REELING:
        // pull the bobber the rest of the way home. award on arrival.
        if (fishing_bobber_reel_toward(&s->bobber, s->rod_tip, SESSION_REEL_SPEED, dt))
            land_catch(s, iw);
        break;

    case CAST_SNAPPED:
        // line's gone; just reset. the catch was already forfeited.
        fishing_session_cancel(s);
        break;

    case CAST_IDLE:
    default:
        break;
    }
}
