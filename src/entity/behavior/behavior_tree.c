#include "behavior_tree.h"
#include "behavior_composite.h"
#include "behavior_decorator.h"
#include "behavior_leaf.h"
#include "../../util/darray.h"
#include <stdio.h>
behavior_tick_fn behavior_tick_for_kind(behavior_node_kind k) {
    switch (k) {
        case BN_SEQUENCE:  return behavior_seq_tick;
        case BN_SELECTOR:  return behavior_sel_tick;
        case BN_PARALLEL:  return behavior_par_tick;
        case BN_INVERTER:  return behavior_inverter_tick;
        case BN_SUCCEEDER: return behavior_succeeder_tick;
        case BN_REPEATER:  return behavior_repeater_tick;
        case BN_RETRY:     return behavior_retry_tick;
        case BN_COOLDOWN:  return behavior_cooldown_tick;
        case BN_CONDITION: return behavior_leaf_tick;
        case BN_ACTION:    return behavior_leaf_tick;
        default:           return NULL;
    }
}

void behavior_tree_init(behavior_tree *t) {
    t->nodes = NULL;
t->root = -1;
t->tick_count = 0;
t->last = BEHAVIOR_INVALID;
behavior_bb_init(&t->bb);
node.last = BEHAVIOR_INVALID;
int idx = (int)darr_len(t->nodes);
darr_push(t->nodes, node);
return idx;
if (!n) return;
n->cursor = 0;
n->counter = 0;
n->last = BEHAVIOR_INVALID;
for (int i = 0;
i < n->child_count;
i++)
        behavior_node_reset(t, n->first_child + i);
if (!n || !n->tick) return BEHAVIOR_FAILURE;
behavior_status s = n->tick(n, ctx);
n->last = s;
return s;
for (int i = 0;
i < depth;
i++) fputs("  ", stdout);
printf("- %s [%s]\n",
           n->name ? n->name : "node",
           behavior_status_name(n->last));
for (int i = 0;
i < n->child_count;
i++)
        dump_rec(t, n->first_child + i, depth + 1);
