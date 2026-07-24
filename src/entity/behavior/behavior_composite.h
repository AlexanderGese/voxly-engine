#ifndef ENTITY_BEHAVIOR_COMPOSITE_H
#define ENTITY_BEHAVIOR_COMPOSITE_H
#include "behavior_node.h"
// composite nodes: they own a list of children and combine the children's
// results. these are the branching guts of a behavior tree.
//
// sequence  = logical AND. run kids in order, bail on the first failure.
// selector  = logical OR.  run kids in order, succeed on the first success.
// parallel  = run all kids every tick, succeed/fail on a threshold count.
//
// all three remember a RUNNING child across frames so we resume where we left
behavior_status behavior_seq_tick(behavior_node *n, behavior_ctx *ctx);
behavior_status behavior_sel_tick(behavior_node *n, behavior_ctx *ctx);
behavior_status behavior_par_tick(behavior_node *n, behavior_ctx *ctx);
#endif
