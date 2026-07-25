#ifndef ENTITY_BEHAVIOR_NODE_H
#define ENTITY_BEHAVIOR_NODE_H

#include "behavior_status.h"
#include "behavior_blackboard.h"
#include <stddef.h>

// a behavior tree node. composites/decorators are the same struct as leaves,
// just with children attached. we don't do a class hierarchy in C; every node
// carries a tick fn pointer and a kind tag for debugging.
//
// all nodes for one tree live in a flat darray owned by behavior_tree, so a
// node references its kids by index, not pointer. that keeps the whole tree in
// one contiguous block and survives the array reallocating as we build it.

struct behavior_tree;       // fwd
struct behavior_node;

// the per-tick context handed to every node. dt in seconds. `agent` is an
// opaque pointer to whatever the game wires up (a voxl_mob*, usually) and
// world is equally opaque so this file stays dependency-free.
typedef struct {
    struct behavior_tree *tree;
    behavior_blackboard  *bb;
    void  *agent;
    void  *world;
    float  dt;
} behavior_ctx;

typedef behavior_status (*behavior_tick_fn)(struct behavior_node *n, behavior_ctx *ctx);

typedef enum {
    BN_INVALID = 0,
    BN_SEQUENCE,
    BN_SELECTOR,
    BN_PARALLEL,
    BN_INVERTER,
    BN_SUCCEEDER,
    BN_REPEATER,
    BN_RETRY,
    BN_COOLDOWN,
    BN_CONDITION,
    BN_ACTION,
    BN_KIND_COUNT
} behavior_node_kind;

// leaf callback. condition leaves return success/failure immediately, action
// leaves may stay RUNNING across frames.
typedef behavior_status (*behavior_leaf_fn)(behavior_ctx *ctx, void *user);

typedef struct behavior_node {
    behavior_node_kind kind;
    behavior_tick_fn   tick;        // resolved at build time from kind

    // children, by index into the tree's node array. -1 == none.
    int   first_child;
    int   child_count;

    // composite/decorator scratch and config, meaning depends on kind.
    int   cursor;                   // which child a sequence/selector is on
    int   counter;                  // repeater/retry loop count
    float timer;                    // cooldown remaining, repeater dwell, etc
    float param;                    // cooldown duration / dwell etc
    int   iparam;                   // repeat target, parallel threshold, etc

    behavior_status last;           // result of the previous tick (debug)

    // leaf payload. ignored by composites.
    behavior_leaf_fn leaf;
    void            *user;
    const char      *name;          // for the debug dumper; may be NULL
} behavior_node;

// reset a node and (recursively, via the tree) its subtree to a fresh run.
void behavior_node_reset(struct behavior_tree *t, int node);

#endif
