#ifndef WORLD_LIGHTPROP_JOB_H
#define WORLD_LIGHTPROP_JOB_H

#include "lightprop_types.h"

// deferred relight scheduler. edits during a busy frame (a player tearing
// through a wall, a fluid tick rewriting a dozen blocks) can stack up more light
// work than we want to do synchronously. callers push edits here and the driver
// drains a bounded number per frame so we never blow the frame budget.
//
// this is the seam the rest of the engine should prefer: fire-and-forget an edit
// and let lightprop catch up. immediate-mode lightprop_on_change is still there
// for the cases that truly need light correct THIS frame (e.g. before a save).

#define LP_JOB_MAX 8192

typedef enum {
    LP_JOB_CHANGE,    // a block at (x,y,z) changed old_id -> new_id
    LP_JOB_CHUNK      // recompute a whole chunk (cx,cz packed into x,z)
} lp_job_kind;

typedef struct {
    lp_job_kind kind;
    int32_t  x, y, z;
    block_id old_id, new_id;
} lp_job;

typedef struct {
    lp_job buf[LP_JOB_MAX];
    int head, tail;
    int dropped;
    // rolling stats so the overlay can show how backed up we are.
    int processed_total;
    int last_frame_count;
} lp_job_queue;

void lp_jobs_init(lp_job_queue *jq);
int  lp_jobs_pending(const lp_job_queue *jq);

// enqueue work. coalescing is the caller's problem; we just FIFO.
void lp_jobs_push_change(lp_job_queue *jq, int wx, int wy, int wz,
                         block_id old_id, block_id new_id);
void lp_jobs_push_chunk(lp_job_queue *jq, int cx, int cz);

// run up to `budget` jobs against the world. returns how many it actually ran.
// pass budget <= 0 to drain everything (use sparingly; can stall).
int  lp_jobs_run(lp_job_queue *jq, world *w, int budget);

#endif
