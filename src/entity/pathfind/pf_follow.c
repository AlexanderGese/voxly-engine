#include "pf_follow.h"

#include <math.h>
#include <string.h>

void pf_follow_init(pf_follow *f, float speed) {
    memset(f, 0, sizeof *f);
    f->state          = PF_FOLLOW_IDLE;
    f->speed          = speed;
    f->arrive_dist    = 1.2f;
    f->waypoint_reach = 0.6f;
    f->stuck_time     = 0.8f;
    f->replan_every   = 1.0f;     // re-plan once a second while walking
    f->replans_left   = 0;
}

void pf_follow_set_goal(pf_follow *f, vec3 goal) {
    f->goal       = goal;
    f->has_goal   = 1;
    f->state      = PF_FOLLOW_PLANNING;
    f->replan_timer = 0.0f;
    f->stuck_timer  = 0.0f;
    f->replans_left = 3;          // a few replans before we admit defeat
}

void pf_follow_clear(pf_follow *f) {
    f->has_goal  = 0;
    f->state     = PF_FOLLOW_IDLE;
    f->path.count = 0;
    f->path.cursor = 0;
    f->path.found = 0;
}

// horizontal distance only. vertical doesn't count toward arrival because a
// mob standing under/over the goal column has effectively arrived.
static float planar_dist(vec3 a, vec3 b) {
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

static vec3 steer_toward(vec3 from, vec3 to, float speed) {
    float dx = to.x - from.x;
    float dz = to.z - from.z;
    float len = sqrtf(dx * dx + dz * dz);
    if (len < 1e-4f) return VEC3_ZERO;
    float inv = speed / len;
    // y left to the physics/jump code; pathing only drives the horizontal.
    return vec3_new(dx * inv, 0.0f, dz * inv);
}

static void try_replan(pf_follow *f, pf_planner *planner, vec3 pos) {
    pf_options opt = pf_options_default();
    if (pf_plan(planner, pos, f->goal, &opt, &f->path) && f->path.found) {
        f->state = PF_FOLLOW_WALKING;
    } else {
        // no path right now. burn a retry; if we're out, give up.
        if (--f->replans_left <= 0) {
            f->state = PF_FOLLOW_STUCK;
        }
        // else stay in PLANNING and try again next tick
    }
}

vec3 pf_follow_tick(pf_follow *f, pf_planner *planner, vec3 pos, float dt) {
    if (!f->has_goal) {
        f->state = PF_FOLLOW_IDLE;
        return VEC3_ZERO;
    }

    // global arrival check, regardless of state.
    if (planar_dist(pos, f->goal) <= f->arrive_dist) {
        f->state = PF_FOLLOW_ARRIVED;
        return VEC3_ZERO;
    }

    f->replan_timer += dt;

    switch (f->state) {
    case PF_FOLLOW_PLANNING:
        try_replan(f, planner, pos);
        if (f->state != PF_FOLLOW_WALKING) return VEC3_ZERO;
        f->last_pos = pos;
        f->stuck_timer = 0.0f;
        /* fall through to walk this same tick */

    case PF_FOLLOW_WALKING: {
        // periodic replan keeps us honest if the target moved or the world
        // changed under us.
        if (f->replan_timer >= f->replan_every) {
            f->replan_timer = 0.0f;
            pf_options opt = pf_options_default();
            pf_path fresh;
            if (pf_plan(planner, pos, f->goal, &opt, &fresh) && fresh.found) {
                f->path = fresh;
            }
        }

        // stuck detection: not enough movement since last sample.
        float moved = planar_dist(pos, f->last_pos);
        if (moved < 0.05f) {
            f->stuck_timer += dt;
            if (f->stuck_timer >= f->stuck_time) {
                f->stuck_timer = 0.0f;
                f->state = PF_FOLLOW_PLANNING;   // shake it off with a replan
                return VEC3_ZERO;
            }
        } else {
            f->stuck_timer = 0.0f;
            f->last_pos = pos;
        }

        if (pf_path_done(&f->path)) {
            // ran out of waypoints but not at goal yet -> replan.
            f->state = PF_FOLLOW_PLANNING;
            return VEC3_ZERO;
        }

        vec3 target = pf_path_step(&f->path, pos, f->waypoint_reach);
        return steer_toward(pos, target, f->speed);
    }

    case PF_FOLLOW_STUCK:
    case PF_FOLLOW_ARRIVED:
    case PF_FOLLOW_IDLE:
    default:
        return VEC3_ZERO;
    }
}

const char *pf_follow_state_name(pf_follow_state s) {
    switch (s) {
    case PF_FOLLOW_IDLE:     return "idle";
    case PF_FOLLOW_PLANNING: return "planning";
    case PF_FOLLOW_WALKING:  return "walking";
    case PF_FOLLOW_STUCK:    return "stuck";
    case PF_FOLLOW_ARRIVED:  return "arrived";
    }
    return "?";
}
