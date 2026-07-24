#include "behavior_builder.h"
#include "behavior_leaf.h"
#include "../../util/darray.h"

// helper: append a node to the tree and, if there's an open parent, the node
// just becomes part of that parent's contiguous child block (we patch counts
// on close). returns the new node's index.
static int emit(behavior_builder *b, behavior_node n) {
    if (!b->ok) return -1;
    return behavior_tree_add(b->tree, n);
}

static void open(behavior_builder *b, behavior_node n) {
    if (!b->ok) return;
    if (b->depth >= BEHAVIOR_BUILD_MAX_DEPTH) { b->ok = 0; return; }
    int idx = emit(b, n);
    b->stack[b->depth] = idx;
    // children start at the next node we push.
    b->start[b->depth] = (int)darr_len(b->tree->nodes);
    b->depth++;
}

void behavior_build_begin(behavior_builder *b, behavior_tree *t) {
    b->tree = t;
    b->depth = 0;
    b->ok = 1;
}

int behavior_build_end(behavior_builder *b) {
    if (!b->ok || b->depth != 0) return -1;     // unbalanced open/close
    if (darr_empty(b->tree->nodes)) return -1;
    // root is the first node we emitted.
    b->tree->root = 0;
    return b->tree->root;
}

void behavior_open_sequence(behavior_builder *b) {
    behavior_node n = {0};
    n.kind = BN_SEQUENCE; n.first_child = -1; n.name = "sequence";
    open(b, n);
}

void behavior_open_selector(behavior_builder *b) {
    behavior_node n = {0};
    n.kind = BN_SELECTOR; n.first_child = -1; n.name = "selector";
    open(b, n);
}

void behavior_open_parallel(behavior_builder *b, int success_threshold) {
    behavior_node n = {0};
    n.kind = BN_PARALLEL; n.first_child = -1; n.name = "parallel";
    n.iparam = success_threshold;
    open(b, n);
}

void behavior_open_inverter(behavior_builder *b) {
    behavior_node n = {0};
    n.kind = BN_INVERTER; n.first_child = -1; n.name = "inverter";
    open(b, n);
}

void behavior_open_succeeder(behavior_builder *b) {
    behavior_node n = {0};
    n.kind = BN_SUCCEEDER; n.first_child = -1; n.name = "succeeder";
    open(b, n);
}

void behavior_open_repeater(behavior_builder *b, int times) {
    behavior_node n = {0};
    n.kind = BN_REPEATER; n.first_child = -1; n.name = "repeater";
    n.iparam = times;
    open(b, n);
}

void behavior_open_retry(behavior_builder *b, int attempts) {
    behavior_node n = {0};
    n.kind = BN_RETRY; n.first_child = -1; n.name = "retry";
    n.iparam = attempts;
    open(b, n);
}

void behavior_open_cooldown(behavior_builder *b, float seconds) {
    behavior_node n = {0};
    n.kind = BN_COOLDOWN; n.first_child = -1; n.name = "cooldown";
    n.param = seconds;
    open(b, n);
}

void behavior_leaf(behavior_builder *b, behavior_leaf_fn fn, void *user, const char *name) {
    emit(b, behavior_make_action(fn, user, name));
}

void behavior_condition(behavior_builder *b, behavior_leaf_fn fn, void *user, const char *name) {
    emit(b, behavior_make_condition(fn, user, name));
}

void behavior_close(behavior_builder *b) {
    if (!b->ok) return;
    if (b->depth <= 0) { b->ok = 0; return; }   // close without open

    b->depth--;
    int parent_idx = b->stack[b->depth];
    int child_start = b->start[b->depth];
    int child_end = (int)darr_len(b->tree->nodes);

    behavior_node *p = behavior_tree_node(b->tree, parent_idx);
    int count = child_end - child_start;

    // decorators must wrap exactly one child. composites need at least one.
    int is_decorator = (p->kind == BN_INVERTER || p->kind == BN_SUCCEEDER ||
                        p->kind == BN_REPEATER || p->kind == BN_RETRY ||
                        p->kind == BN_COOLDOWN);
    if (count <= 0) { b->ok = 0; return; }
    if (is_decorator && count != 1) { b->ok = 0; return; }

    p->first_child = child_start;
    p->child_count = count;
}
