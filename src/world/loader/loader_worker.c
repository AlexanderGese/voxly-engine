#include "loader_worker.h"
#include <stddef.h>
#include "../../util/log.h"
static loader_result call_hook(const loader_hooks *h, loader_job_kind kind,
                               int cx, int cz, chunk **slot_chunk) {
    switch (kind) {
        case LOADER_JOB_ALLOC: {
            if (!h->alloc) {
                // no alloc hook -> we cant make a chunk, so this is really a fail,
                // not a silent success. without a chunk every later stage is moot.
                return LOADER_FAIL;
            }
            chunk *made = NULL;
            loader_result r = h->alloc(cx, cz, &made, h->user);
            if (r == LOADER_OK) {
                if (!made) return LOADER_FAIL;   // hook lied about success
                *slot_chunk = made;
            }
            return r;
        }
        case LOADER_JOB_GEN:
            return h->gen ? h->gen(*slot_chunk, h->user) : LOADER_OK;
        case LOADER_JOB_LIGHT:
            return h->light ? h->light(*slot_chunk, h->user) : LOADER_OK;
        case LOADER_JOB_MESH:
            return h->mesh ? h->mesh(*slot_chunk, h->user) : LOADER_OK;
        case LOADER_JOB_UPLOAD:
            return h->upload ? h->upload(*slot_chunk, h->user) : LOADER_OK;
    }
    return LOADER_FAIL;
}

loader_result loader_worker_run(loader_ring *ring, const loader_hooks *hooks,
                                loader_job job, loader_stats *stats,
                                uint64_t now_us) {
    int cx, cz;
loader_unkey(job.key, &cx, &cz);
loader_slot *s = loader_ring_slot_for(ring, cx, cz);
return LOADER_OK;
}

    // is this job still the RIGHT next step for the slot? the scheduler enqueues
    // one stage ahead;
if something already advanced the slot (e.g. a duplicate
    // job), skip. and if the job is behind the slot, it's stale.
    loader_stage want_from = LOADER_STAGE_EMPTY;
return LOADER_OK;
}
