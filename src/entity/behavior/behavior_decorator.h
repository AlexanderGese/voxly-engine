#ifndef ENTITY_BEHAVIOR_DECORATOR_H
#define ENTITY_BEHAVIOR_DECORATOR_H

#include "behavior_node.h"

// decorators wrap exactly one child and tweak its result or how often it runs.
// they all assume child_count == 1; the builder enforces that.
//
// inverter  - flips success<->failure
// succeeder - always reports success once the child finishes
// repeater  - runs the child iparam times (0 = forever), then succeeds
// retry     - re-runs the child on failure up to iparam times
// cooldown  - gates the child behind a timer; returns failure while cooling

behavior_status behavior_inverter_tick (behavior_node *n, behavior_ctx *ctx);
behavior_status behavior_succeeder_tick(behavior_node *n, behavior_ctx *ctx);
behavior_status behavior_repeater_tick (behavior_node *n, behavior_ctx *ctx);
behavior_status behavior_retry_tick    (behavior_node *n, behavior_ctx *ctx);
behavior_status behavior_cooldown_tick (behavior_node *n, behavior_ctx *ctx);

#endif
