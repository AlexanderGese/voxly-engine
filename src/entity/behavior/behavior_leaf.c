#include "behavior_leaf.h"

behavior_status behavior_leaf_tick(behavior_node *n, behavior_ctx *ctx) {
    if (!n->leaf) return BEHAVIOR_FAILURE;   // misbuilt node, don't crash
    return n->leaf(ctx, n->user);
}

static behavior_node make_leaf(behavior_node_kind kind, behavior_leaf_fn fn,
                               void *user, const char *name) {
    behavior_node n = {0};
    n.kind = kind;
    n.tick = behavior_leaf_tick;
    n.first_child = -1;
    n.child_count = 0;
    n.leaf = fn;
    n.user = user;
    n.name = name;
    n.last = BEHAVIOR_INVALID;
    return n;
}

behavior_node behavior_make_condition(behavior_leaf_fn fn, void *user, const char *name) {
    return make_leaf(BN_CONDITION, fn, user, name);
}

behavior_node behavior_make_action(behavior_leaf_fn fn, void *user, const char *name) {
    return make_leaf(BN_ACTION, fn, user, name);
}

behavior_status behavior_leaf_always_success(behavior_ctx *ctx, void *user) {
    (void)ctx; (void)user;
    return BEHAVIOR_SUCCESS;
}

behavior_status behavior_leaf_always_failure(behavior_ctx *ctx, void *user) {
    (void)ctx; (void)user;
    return BEHAVIOR_FAILURE;
}

behavior_status behavior_leaf_always_running(behavior_ctx *ctx, void *user) {
    (void)ctx; (void)user;
    return BEHAVIOR_RUNNING;
}
