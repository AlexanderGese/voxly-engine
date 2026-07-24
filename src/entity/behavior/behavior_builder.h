#ifndef ENTITY_BEHAVIOR_BUILDER_H
#define ENTITY_BEHAVIOR_BUILDER_H

#include "behavior_tree.h"
#include "behavior_node.h"

// fluent-ish builder for trees. because nodes reference children by index and
// the node array grows as we push, we can't take a parent's address up front.
// instead the builder keeps a stack of "open" composites/decorators and
// patches their child range when they close.
//
// usage:
// behavior_builder b;
// behavior_build_begin(&b, tree);
// behavior_open_selector(&b);
// behavior_open_sequence(&b);
// behavior_leaf(&b, see_target_fn,  NULL, "see?");
// behavior_leaf(&b, chase_fn,       NULL, "chase");
// behavior_close(&b);
// behavior_leaf(&b, wander_fn, NULL, "wander");
// behavior_close(&b);
// behavior_build_end(&b);
//
// children of a composite must be added contiguously (they are, since the
// builder pushes them straight after opening), so first_child + count covers
// exactly that composite's kids. nesting works because each open node records
// where its children start.

#define BEHAVIOR_BUILD_MAX_DEPTH 32

typedef struct {
    behavior_tree *tree;
    int   stack[BEHAVIOR_BUILD_MAX_DEPTH];   // open node indices
    int   start[BEHAVIOR_BUILD_MAX_DEPTH];   // node count when each was opened
    int   depth;
    int   ok;                                // cleared on any error
} behavior_builder;

void behavior_build_begin(behavior_builder *b, behavior_tree *t);
int  behavior_build_end(behavior_builder *b);   // returns root index, -1 on err

// composites
void behavior_open_sequence(behavior_builder *b);
void behavior_open_selector(behavior_builder *b);
void behavior_open_parallel(behavior_builder *b, int success_threshold);

// decorators (single child; close after adding exactly one)
void behavior_open_inverter (behavior_builder *b);
void behavior_open_succeeder(behavior_builder *b);
void behavior_open_repeater (behavior_builder *b, int times);   // 0 = forever
void behavior_open_retry    (behavior_builder *b, int attempts);
void behavior_open_cooldown (behavior_builder *b, float seconds);

// leaves
void behavior_leaf     (behavior_builder *b, behavior_leaf_fn fn, void *user, const char *name);
void behavior_condition(behavior_builder *b, behavior_leaf_fn fn, void *user, const char *name);

// close the innermost open node, wiring up its children.
void behavior_close(behavior_builder *b);

#endif
