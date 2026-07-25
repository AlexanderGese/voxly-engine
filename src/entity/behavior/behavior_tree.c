#include "behavior_tree.h"
#include "behavior_composite.h"
#include "behavior_decorator.h"
#include "behavior_leaf.h"
#include "../../util/darray.h"

#include <stdio.h>

// resolve a kind to its tick function. centralised here so the builder only
// has to set .kind and we fill in the rest. keeps the two in sync.
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
}

void behavior_tree_free(behavior_tree *t) {
    darr_free(t->nodes);
    behavior_bb_free(&t->bb);
    t->root = -1;
}

int behavior_tree_add(behavior_tree *t, behavior_node node) {
    // fill in the tick fn from the kind if the builder left it null.
    if (!node.tick) node.tick = behavior_tick_for_kind(node.kind);
    node.last = BEHAVIOR_INVALID;
    int idx = (int)darr_len(t->nodes);
    darr_push(t->nodes, node);
    return idx;
}

behavior_node *behavior_tree_node(behavior_tree *t, int idx) {
    if (idx < 0 || idx >= (int)darr_len(t->nodes)) return NULL;
    return &t->nodes[idx];
}

void behavior_node_reset(behavior_tree *t, int idx) {
    behavior_node *n = behavior_tree_node(t, idx);
    if (!n) return;
    n->cursor = 0;
    n->counter = 0;
    n->last = BEHAVIOR_INVALID;
    // cooldown timers intentionally survive a reset; they model real elapsed
    // time, not run state. everything else is run state.
    for (int i = 0; i < n->child_count; i++)
        behavior_node_reset(t, n->first_child + i);
}

void behavior_tree_reset(behavior_tree *t) {
    if (t->root >= 0) behavior_node_reset(t, t->root);
    t->last = BEHAVIOR_INVALID;
}

behavior_status behavior_tree_tick_child(behavior_tree *t, int child, behavior_ctx *ctx) {
    behavior_node *n = behavior_tree_node(t, child);
    if (!n || !n->tick) return BEHAVIOR_FAILURE;
    behavior_status s = n->tick(n, ctx);
    n->last = s;
    return s;
}

behavior_status behavior_tree_tick(behavior_tree *t, void *agent, void *world, float dt) {
    if (t->root < 0) return BEHAVIOR_FAILURE;

    behavior_ctx ctx = {
        .tree  = t,
        .bb    = &t->bb,
        .agent = agent,
        .world = world,
        .dt    = dt,
    };

    behavior_status s = behavior_tree_tick_child(t, t->root, &ctx);
    t->last = s;
    t->tick_count++;

    // a finished root means the run is over; reset so we start clean next time.
    // running stays put so leaves keep their cross-frame state.
    if (behavior_status_is_done(s))
        behavior_tree_reset(t);

    return s;
}

static void dump_rec(const behavior_tree *t, int idx, int depth) {
    const behavior_node *n = &t->nodes[idx];
    for (int i = 0; i < depth; i++) fputs("  ", stdout);
    printf("- %s [%s]\n",
           n->name ? n->name : "node",
           behavior_status_name(n->last));
    for (int i = 0; i < n->child_count; i++)
        dump_rec(t, n->first_child + i, depth + 1);
}

void behavior_tree_dump(const behavior_tree *t) {
    if (t->root < 0) { printf("(empty tree)\n"); return; }
    printf("tree: %d nodes, %d ticks, root=%s\n",
           (int)darr_len(t->nodes), t->tick_count,
           behavior_status_name(t->last));
    dump_rec(t, t->root, 0);
}
