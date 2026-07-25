#include "behavior_composite.h"
#include "behavior_tree.h"

// sequence: tick children left to right. cursor remembers which child is
// still running so we don't redo the ones that already passed. resets cursor
// to 0 on completion so the next run starts fresh.
behavior_status behavior_seq_tick(behavior_node *n, behavior_ctx *ctx) {
    behavior_tree *t = ctx->tree;

    while (n->cursor < n->child_count) {
        int child = n->first_child + n->cursor;
        behavior_status s = behavior_tree_tick_child(t, child, ctx);

        if (s == BEHAVIOR_RUNNING)
            return BEHAVIOR_RUNNING;        // resume here next frame

        if (s == BEHAVIOR_FAILURE) {
            n->cursor = 0;                  // whole sequence failed, rewind
            return BEHAVIOR_FAILURE;
        }
        // success: advance to the next child
        n->cursor++;
    }

    // ran off the end -> every child succeeded
    n->cursor = 0;
    return BEHAVIOR_SUCCESS;
}

// selector: mirror image of sequence. first success wins, only fails if every
// child fails.
behavior_status behavior_sel_tick(behavior_node *n, behavior_ctx *ctx) {
    behavior_tree *t = ctx->tree;

    while (n->cursor < n->child_count) {
        int child = n->first_child + n->cursor;
        behavior_status s = behavior_tree_tick_child(t, child, ctx);

        if (s == BEHAVIOR_RUNNING)
            return BEHAVIOR_RUNNING;

        if (s == BEHAVIOR_SUCCESS) {
            n->cursor = 0;
            return BEHAVIOR_SUCCESS;
        }
        // failure: try the next option
        n->cursor++;
    }

    n->cursor = 0;
    return BEHAVIOR_FAILURE;
}

// parallel: tick every non-finished child each frame. iparam is the number of
// successes required to succeed. once enough succeed -> success; once it's
// impossible to reach the threshold -> failure. otherwise running.
//
// we reuse counter to count successes-so-far and cursor as a failure count.
// children that already finished get skipped via the per-node `last` field.
behavior_status behavior_par_tick(behavior_node *n, behavior_ctx *ctx) {
    behavior_tree *t = ctx->tree;

    int need = n->iparam > 0 ? n->iparam : n->child_count;   // default: all
    int succ = 0, fail = 0, running = 0;

    for (int i = 0; i < n->child_count; i++) {
        int child = n->first_child + i;
        behavior_node *cn = behavior_tree_node(t, child);

        // skip children that already settled this run
        if (cn->last == BEHAVIOR_SUCCESS) { succ++; continue; }
        if (cn->last == BEHAVIOR_FAILURE) { fail++; continue; }

        behavior_status s = behavior_tree_tick_child(t, child, ctx);
        if (s == BEHAVIOR_SUCCESS)      succ++;
        else if (s == BEHAVIOR_FAILURE) fail++;
        else                            running++;
    }

    if (succ >= need) {
        behavior_node_reset(t, (int)(n - t->nodes));   // clear child results
        return BEHAVIOR_SUCCESS;
    }
    // not enough children remain that could still succeed
    if (succ + running < need) {
        behavior_node_reset(t, (int)(n - t->nodes));
        return BEHAVIOR_FAILURE;
    }
    return BEHAVIOR_RUNNING;
}
