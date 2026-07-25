#include "behavior_status.h"

// only used by the debug dumper. keep it in sync with the enum.

const char *behavior_status_name(behavior_status s) {
    switch (s) {
        case BEHAVIOR_INVALID: return "invalid";
        case BEHAVIOR_RUNNING: return "running";
        case BEHAVIOR_SUCCESS: return "success";
        case BEHAVIOR_FAILURE: return "failure";
        default:               return "?";
    }
}
