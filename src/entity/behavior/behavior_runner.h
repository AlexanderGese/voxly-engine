#ifndef ENTITY_BEHAVIOR_RUNNER_H
#define ENTITY_BEHAVIOR_RUNNER_H

#include "behavior_tree.h"
#include <stdint.h>

// owns and ticks a pile of trees, one per active mob. the world doesn't want
// to tick every behavior tree every frame when there are hundreds of mobs
// loaded, so the runner does round-robin time-slicing: each registered tree
// gets ticked at most once per `interval` seconds, and we spread the work so
// roughly 1/interval-th of the trees run on any given frame.
//
// trees keep accumulating dt between their own ticks so a tree that runs at
// 5hz still sees the full elapsed time, not a single frame's worth.

typedef struct {
    uint32_t       mob_id;      // who this tree belongs to
    behavior_tree  tree;
    void          *agent;       // voxl_mob*, cached so tick is cheap
    float          accum;       // dt banked since last tick
    int            active;      // 0 = slot free / despawned
} behavior_slot;

typedef struct {
    behavior_slot *slots;       // darray
    void  *world;               // opaque, forwarded into every ctx
    float  interval;            // seconds between ticks of a single tree
    int    cursor;              // round-robin position
    int    max_per_frame;       // hard cap on ticks per update, 0 = unlimited
    int    active_count;
} behavior_runner;

void behavior_runner_init(behavior_runner *r, void *world, float interval);
void behavior_runner_free(behavior_runner *r);

// register a tree for a mob. the runner takes ownership of *tree (shallow:
// it copies the struct, so don't free the original separately). returns the
// slot index, or -1 on failure.
int  behavior_runner_add(behavior_runner *r, uint32_t mob_id, behavior_tree tree, void *agent);

// drop a mob's tree (despawn). frees the tree. safe to call with an unknown id.
void behavior_runner_remove(behavior_runner *r, uint32_t mob_id);

// look up a live tree by mob id so the game can poke its blackboard (set the
// target point each frame, read "did_attack", etc). NULL if not registered.
behavior_tree *behavior_runner_tree(behavior_runner *r, uint32_t mob_id);

// advance the whole pile by dt. ticks the time-sliced subset of trees.
// returns how many trees actually ticked this frame.
int  behavior_runner_update(behavior_runner *r, float dt);

#endif
