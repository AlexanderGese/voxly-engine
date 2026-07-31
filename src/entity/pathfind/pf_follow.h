#ifndef ENTITY_PATHFIND_PF_FOLLOW_H
#define ENTITY_PATHFIND_PF_FOLLOW_H

#include "pf.h"
#include "../../math/vec3.h"

// path-following state machine the AI hangs off an entity. it owns a path,
// knows when to replan, and turns waypoints into a desired velocity the mob
// controller can feed into its physics. keeps the actual ai code dumb.

typedef enum {
    PF_FOLLOW_IDLE = 0,    // no goal
    PF_FOLLOW_PLANNING,    // goal set, needs a (re)plan this tick
    PF_FOLLOW_WALKING,     // following the current path
    PF_FOLLOW_STUCK,       // not making progress, give up soon
    PF_FOLLOW_ARRIVED      // close enough to the goal
} pf_follow_state;

typedef struct {
    pf_path         path;
    vec3            goal;
    int             has_goal;
    pf_follow_state state;

    float           speed;          // how fast we want to move (blocks/s)
    float           arrive_dist;    // within this of goal -> arrived
    float           waypoint_reach; // within this of a waypoint -> advance

    // stuck detection. if we haven't moved much in `stuck_time` we replan
    // once, and if that doesn't help we bail to IDLE.
    vec3            last_pos;
    float           stuck_timer;
    float           stuck_time;
    float           replan_timer;
    float           replan_every;   // periodic replan to chase moving targets
    int             replans_left;
} pf_follow;

// init with sensible defaults for the given move speed.
void pf_follow_init(pf_follow *f, float speed);

// set / clear the goal. setting a goal flips state to PLANNING.
void pf_follow_set_goal(pf_follow *f, vec3 goal);
void pf_follow_clear(pf_follow *f);

// advance the state machine. `pos` is the entity position, `dt` the frame
// time. returns a desired velocity (length up to `speed`). may call into the
// planner when a (re)plan is due. on STUCK/ARRIVED returns a zero vector.
vec3 pf_follow_tick(pf_follow *f, pf_planner *planner, vec3 pos, float dt);

const char *pf_follow_state_name(pf_follow_state s);

#endif
