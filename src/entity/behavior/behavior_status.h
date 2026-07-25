#ifndef ENTITY_BEHAVIOR_STATUS_H
#define ENTITY_BEHAVIOR_STATUS_H

// the three classic bt return codes plus an "invalid" sentinel for nodes
// that haven't been ticked yet this run. every node tick spits one of these
// back up to its parent and the parent decides what to do with it.

typedef enum {
    BEHAVIOR_INVALID = 0,   // never ticked / freshly reset
    BEHAVIOR_RUNNING,       // still working, tick me again next frame
    BEHAVIOR_SUCCESS,
    BEHAVIOR_FAILURE,
} behavior_status;

static inline int behavior_status_is_done(behavior_status s) {
    return s == BEHAVIOR_SUCCESS || s == BEHAVIOR_FAILURE;
}

// flip success<->failure. handy for the inverter decorator. running and
// invalid pass through untouched.
static inline behavior_status behavior_status_invert(behavior_status s) {
    if (s == BEHAVIOR_SUCCESS) return BEHAVIOR_FAILURE;
    if (s == BEHAVIOR_FAILURE) return BEHAVIOR_SUCCESS;
    return s;
}

const char *behavior_status_name(behavior_status s);

#endif
