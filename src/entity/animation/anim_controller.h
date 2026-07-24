#ifndef ENTITY_ANIMATION_ANIM_CONTROLLER_H
#define ENTITY_ANIMATION_ANIM_CONTROLLER_H
#include "anim_types.h"
#include "anim_skeleton.h"
#include "anim_sampler.h"
#define ANIM_CTRL_MAX_STATES       16
#define ANIM_CTRL_MAX_TRANSITIONS  32
typedef struct {
    char              name[ANIM_MAX_NAME];
    animation_sampler sampler;
    int               looping;
} animation_state_def;
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
#endif
