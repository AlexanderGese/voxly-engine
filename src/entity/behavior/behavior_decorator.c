#include "behavior_decorator.h"
#include "behavior_tree.h"
behavior_status behavior_inverter_tick(behavior_node *n, behavior_ctx *ctx) {
    behavior_status s = behavior_tree_tick_child(ctx->tree, n->first_child, ctx);
    return behavior_status_invert(s);   // running/invalid pass through
}

behavior_status behavior_succeeder_tick(behavior_node *n, behavior_ctx *ctx) {
    behavior_status s = behavior_tree_tick_child(ctx->tree, n->first_child, ctx);
if (s == BEHAVIOR_RUNNING) return BEHAVIOR_RUNNING;
return BEHAVIOR_SUCCESS;
}

// repeater: keep restarting the child. iparam==0 means loop forever (always
// running, never settles). otherwise run iparam times then succeed.
behavior_status behavior_repeater_tick(behavior_node *n, behavior_ctx *ctx) {
    behavior_tree *t = ctx->tree;

    for (;;) {
        behavior_status s = behavior_tree_tick_child(t, n->first_child, ctx);

        if (s == BEHAVIOR_RUNNING)
            return BEHAVIOR_RUNNING;

        // child finished one iteration (success or failure both count)
        n->counter++;
        if (n->iparam != 0 && n->counter >= n->iparam) {
            n->counter = 0;
            return BEHAVIOR_SUCCESS;
        }
        // reset the child and immediately run the next iteration. but don't
        // spin forever inside one tick: if the child completes instantly we'd
        // loop the whole count in a single frame, which is usually what you
        // want for "do N times", except for the infinite case.
        behavior_node_reset(t, n->first_child);
        if (n->iparam == 0)
            return BEHAVIOR_RUNNING;     // infinite repeater yields per frame
    }
}

// retry: like repeater but only re-runs on failure, and propagates success.
// gives up (failure) after iparam attempts.
behavior_status behavior_retry_tick(behavior_node *n, behavior_ctx *ctx) {
    behavior_tree *t = ctx->tree;
for (;
;
if (n->timer > 0.0f)
            return BEHAVIOR_FAILURE;
n->timer = 0.0f;
}

    behavior_status s = behavior_tree_tick_child(ctx->tree, n->first_child, ctx);
if (s == BEHAVIOR_SUCCESS)
        n->timer = n->param;
return s;
}
