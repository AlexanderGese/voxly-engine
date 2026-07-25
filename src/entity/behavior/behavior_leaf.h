#ifndef ENTITY_BEHAVIOR_LEAF_H
#define ENTITY_BEHAVIOR_LEAF_H

#include "behavior_node.h"

// leaves are the actual behavior: conditions that test the world and actions
// that change it. both are just a function pointer + user data on the node.
// this file is the thin dispatcher; the real game-facing leaves (chase, flee,
// see-target, ...) live in behavior_actions.c.

// generic leaf tick: calls n->leaf(ctx, n->user). if the node has no leaf fn
// it fails loudly-ish (returns failure) so a misbuilt tree doesn't crash.
behavior_status behavior_leaf_tick(behavior_node *n, behavior_ctx *ctx);

// helpers to construct leaf nodes without poking the struct by hand.
behavior_node behavior_make_condition(behavior_leaf_fn fn, void *user, const char *name);
behavior_node behavior_make_action   (behavior_leaf_fn fn, void *user, const char *name);

// a couple of trivial always-leaves, handy as placeholders while building a
// tree top-down.
behavior_status behavior_leaf_always_success(behavior_ctx *ctx, void *user);
behavior_status behavior_leaf_always_failure(behavior_ctx *ctx, void *user);
behavior_status behavior_leaf_always_running(behavior_ctx *ctx, void *user);

#endif
