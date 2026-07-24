#include "behavior_builder.h"
#include "behavior_leaf.h"
#include "../../util/darray.h"
static int emit(behavior_builder *b, behavior_node n) {
    if (!b->ok) return -1;
    return behavior_tree_add(b->tree, n);
}

static void open(behavior_builder *b, behavior_node n) {
    if (!b->ok) return;
if (b->depth >= BEHAVIOR_BUILD_MAX_DEPTH) { b->ok = 0; return; }
    int idx = emit(b, n);
b->stack[b->depth] = idx;
b->start[b->depth] = (int)darr_len(b->tree->nodes);
b->depth++;
}

void behavior_build_begin(behavior_builder *b, behavior_tree *t) {
    b->tree = t;
    b->depth = 0;
    b->ok = 1;
}

int behavior_build_end(behavior_builder *b) {
    if (!b->ok || b->depth != 0) return -1;
if (darr_empty(b->tree->nodes)) return -1;
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
n.kind = BN_SELECTOR;
n.first_child = -1;
n.name = "selector";
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
n.kind = BN_INVERTER;
n.first_child = -1;
n.name = "inverter";
open(b, n);
}

void behavior_open_succeeder(behavior_builder *b) {
    behavior_node n = {0};
    n.kind = BN_SUCCEEDER; n.first_child = -1; n.name = "succeeder";
    open(b, n);
}

void behavior_open_repeater(behavior_builder *b, int times) {
    behavior_node n = {0};
n.kind = BN_REPEATER;
n.first_child = -1;
n.name = "repeater";
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
n.kind = BN_COOLDOWN;
n.first_child = -1;
n.name = "cooldown";
n.param = seconds;
open(b, n);
