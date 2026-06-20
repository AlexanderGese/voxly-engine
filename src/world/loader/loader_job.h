#ifndef WORLD_LOADER_JOB_H
#define WORLD_LOADER_JOB_H

#include <stdint.h>
#include "loader_types.h"

// one unit of deferred work. the queue stores these by value (no pointer chasing
// in the heap), so keep it small and trivially copyable. the chunk it targets is
// addressed by coord, not pointer -- the chunk might not even exist yet (ALLOC
// jobs) and we dont want a dangling ptr if the world unloads it under us.

typedef struct {
    uint64_t key;        // chunk coord, see loader_key
    loader_job_kind kind;
    int      priority;   // lower = more urgent. computed at enqueue time.
    uint32_t serial;     // tiebreaker / staleness check, see below
    uint64_t enqueued_us;// when it went in. used to age out stale jobs.
} loader_job;

// serials: every time we enqueue work for a chunk we stamp it with the chunk's
// current "generation". if the chunk gets unloaded and a new one takes its slot
// the generation bumps, and any old job still sitting in the queue is detected as
// stale and dropped when popped. cheap ABA guard without a free-then-reuse race.

loader_job loader_job_make(uint64_t key, loader_job_kind kind,
                           int priority, uint32_t serial, uint64_t now_us);

// the stage a job moves a chunk INTO if it succeeds. handy for the worker so it
// doesnt carry a parallel switch.
loader_stage loader_job_target_stage(loader_job_kind kind);

// which job kind advances a chunk currently sitting at `stage`. returns -1 when
// the stage is terminal (RESIDENT) and there's nothing left to schedule.
int loader_job_for_stage(loader_stage stage);

// jobs that must run on the main thread (touch GL). the worker refuses to run
// these off the main pump. right now thats just UPLOAD.
int loader_job_is_main_thread(loader_job_kind kind);

// human label, for logs. no allocation, returns a static string.
const char *loader_job_name(loader_job_kind kind);

#endif
