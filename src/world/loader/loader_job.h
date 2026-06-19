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
loader_job loader_job_make(uint64_t key, loader_job_kind kind,
                           int priority, uint32_t serial, uint64_t now_us);
loader_stage loader_job_target_stage(loader_job_kind kind);
int loader_job_for_stage(loader_stage stage);
int loader_job_is_main_thread(loader_job_kind kind);
const char *loader_job_name(loader_job_kind kind);
#endif
