#ifndef WORLD_LOADER_JOB_H
#define WORLD_LOADER_JOB_H
#include <stdint.h>
#include "loader_types.h"
typedef struct {
    uint64_t key;        // chunk coord, see loader_key
    loader_job_kind kind;
    int      priority;   // lower = more urgent. computed at enqueue time.
    uint32_t serial;     // tiebreaker / staleness check, see below
    uint64_t enqueued_us;// when it went in. used to age out stale jobs.
} loader_job;
#endif
