#include "lightprop_job.h"
#include "lightprop.h"
#include "../../util/log.h"

void lp_jobs_init(lp_job_queue *jq) {
    jq->head = jq->tail = 0;
    jq->dropped = 0;
    jq->processed_total = 0;
    jq->last_frame_count = 0;
}

int lp_jobs_pending(const lp_job_queue *jq) {
    int n = jq->tail - jq->head;
    if (n < 0) n += LP_JOB_MAX;
    return n;
}

static void push(lp_job_queue *jq, const lp_job *j) {
    int nt = jq->tail + 1;
    if (nt >= LP_JOB_MAX) nt = 0;
    if (nt == jq->head) {
        // full. dropping a relight job means a stale lit region until the next
        // edit there; log loudly because it usually means a runaway caller.
        jq->dropped++;
        if ((jq->dropped & 1023) == 1)
            LOGW("lightprop: job queue full, dropping relight work (%d total)",
                 jq->dropped);
        return;
    }
    jq->buf[jq->tail] = *j;
    jq->tail = nt;
}

void lp_jobs_push_change(lp_job_queue *jq, int wx, int wy, int wz,
                         block_id old_id, block_id new_id) {
    if (old_id == new_id) return;
    lp_job j = { LP_JOB_CHANGE, wx, wy, wz, old_id, new_id };
    push(jq, &j);
}

void lp_jobs_push_chunk(lp_job_queue *jq, int cx, int cz) {
    // stuff the chunk address into x/z; y unused for this kind.
    lp_job j = { LP_JOB_CHUNK, cx, 0, cz, 0, 0 };
    push(jq, &j);
}

static int pop(lp_job_queue *jq, lp_job *out) {
    if (jq->head == jq->tail) return 0;
    *out = jq->buf[jq->head];
    int nh = jq->head + 1;
    if (nh >= LP_JOB_MAX) nh = 0;
    jq->head = nh;
    return 1;
}

int lp_jobs_run(lp_job_queue *jq, world *w, int budget) {
    int done = 0;
    lp_job j;
    while ((budget <= 0 || done < budget) && pop(jq, &j)) {
        switch (j.kind) {
            case LP_JOB_CHANGE:
                lightprop_on_change(w, j.x, j.y, j.z, j.old_id, j.new_id);
                break;
            case LP_JOB_CHUNK: {
                chunk *c = world_get_chunk(w, j.x, j.z);
                if (c) lightprop_chunk_full(w, c);
                break;
            }
        }
        done++;
    }
    jq->processed_total += done;
    jq->last_frame_count = done;
    return done;
}
