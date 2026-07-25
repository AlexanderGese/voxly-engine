#ifndef ENTITY_BEHAVIOR_TREE_H
#define ENTITY_BEHAVIOR_TREE_H

#include "behavior_node.h"
#include "behavior_blackboard.h"
#include "behavior_status.h"

// owns the flat node array and the root index. one tree per behaving entity,
// or shared read-only if you keep the runtime cursors elsewhere (we don't, we
// just give each mob its own tree clone, trees are tiny).
//
// build a tree with the builder in behavior_builder.h, then call
// behavior_tree_tick once a frame.

typedef struct behavior_tree {
    behavior_node *nodes;       // darray
    int            root;        // index of root, -1 if empty
    behavior_blackboard bb;     // tree-local blackboard

    int   tick_count;           // total ticks run, for debugging
    behavior_status last;       // root result last tick
} behavior_tree;

void behavior_tree_init(behavior_tree *t);
void behavior_tree_free(behavior_tree *t);

// push a node into the array, returns its index. used by the builder.
int  behavior_tree_add(behavior_tree *t, behavior_node node);
behavior_node *behavior_tree_node(behavior_tree *t, int idx);

// run one frame. agent/world are passed straight into the ctx. returns the
// root's status. resets the tree automatically when the root finishes so the
// next tick starts a fresh run.
behavior_status behavior_tree_tick(behavior_tree *t, void *agent, void *world, float dt);

// force a clean run on the next tick (e.g. after the mob changes state).
void behavior_tree_reset(behavior_tree *t);

// dump the tree to stdout with indentation and last-status per node. debug.
void behavior_tree_dump(const behavior_tree *t);

// dispatch helper used by composites to tick a child by index.
behavior_status behavior_tree_tick_child(behavior_tree *t, int child, behavior_ctx *ctx);

#endif
