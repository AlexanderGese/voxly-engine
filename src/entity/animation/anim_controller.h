#ifndef ENTITY_ANIMATION_ANIM_CONTROLLER_H
#define ENTITY_ANIMATION_ANIM_CONTROLLER_H

#include "anim_types.h"
#include "anim_skeleton.h"
#include "anim_sampler.h"

// the animation state machine. each state owns a clip + loop flag; transitions
// cross-fade from the old state's last pose to the new one over a fixed time.
// this is the top-level thing an entity actually pokes: set a state, tick it,
// read back a pose. it stitches sampler + pose + blend together.

#define ANIM_CTRL_MAX_STATES       16
#define ANIM_CTRL_MAX_TRANSITIONS  32

typedef struct {
    char              name[ANIM_MAX_NAME];
    animation_sampler sampler;
    int               looping;
} animation_state_def;

// an automatic transition: when in `from` (or any state if from<0) and the
// trigger id is fired, slide to `to` over `duration` seconds. lets gameplay
// fire symbolic events instead of naming target states everywhere.
typedef struct {
    int   from;        // source state index, or -1 for "any"
    int   to;          // destination state index
    int   trigger;     // user-defined event id matched in fire()
    float duration;    // cross-fade seconds
} animation_transition;

typedef struct {
    const animation_skeleton *skeleton;

    animation_state_def states[ANIM_CTRL_MAX_STATES];
    int state_count;

    animation_transition transitions[ANIM_CTRL_MAX_TRANSITIONS];
    int transition_count;

    int   current;      // active state index, or -1 before first set
    int   previous;     // state we're fading out of during a transition
    float fade_t;       // 0..1 progress of the active cross-fade
    float fade_dur;     // length of the active cross-fade (seconds)
    int   fading;       // 1 while a cross-fade is in flight

    animation_pose rest;       // cached rest pose, the eval seed
    animation_pose prev_pose;  // frozen pose of the outgoing state
    animation_pose out_pose;   // latest evaluated result (read this)
} animation_controller;

void animation_controller_init(animation_controller *c,
                               const animation_skeleton *sk);

// register a state. returns its index, or -1 if the table is full.
int  animation_controller_add_state(animation_controller *c, const char *name,
                                    const animation_clip *clip, int looping);

int  animation_controller_find_state(const animation_controller *c,
                                     const char *name);

void animation_controller_add_transition(animation_controller *c, int from,
                                         int to, int trigger, float duration);

// immediately switch (no fade). mostly for the initial state.
void animation_controller_set_state(animation_controller *c, int state);

// begin a cross-fade to `state` over `duration` seconds.
void animation_controller_play(animation_controller *c, int state, float duration);

// fire a trigger id; if a matching transition exists for the current (or any)
// state, it kicks off. returns 1 if a transition started.
int  animation_controller_fire(animation_controller *c, int trigger);

// advance time, progress any cross-fade, and bake out_pose. read out_pose after.
void animation_controller_update(animation_controller *c, float dt);

// convenience: latest evaluated pose.
const animation_pose *animation_controller_pose(const animation_controller *c);

#endif
