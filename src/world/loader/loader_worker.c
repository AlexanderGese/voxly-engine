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
if (!s) {
        // chunk rolled off the ring while the job waited. drop it.
        if (stats) stats->stale_dropped++;
        return LOADER_OK;
    }
    if (s->gen != job.serial) {
        // slot got rehomed (ABA). the job belongs to a previous occupant.
        if (stats) stats->stale_dropped++;
return LOADER_OK;
}

    // is this job still the RIGHT next step for the slot? the scheduler enqueues
    // one stage ahead;
if something already advanced the slot (e.g. a duplicate
    // job), skip. and if the job is behind the slot, it's stale.
    loader_stage want_from = LOADER_STAGE_EMPTY;
switch (job.kind) {
        case LOADER_JOB_ALLOC:  want_from = LOADER_STAGE_EMPTY;     break;
        case LOADER_JOB_GEN:    want_from = LOADER_STAGE_ALLOCED;   break;
        case LOADER_JOB_LIGHT:  want_from = LOADER_STAGE_GENERATED; break;
        case LOADER_JOB_MESH:   want_from = LOADER_STAGE_LIT;       break;
        case LOADER_JOB_UPLOAD: want_from = LOADER_STAGE_MESHED;    break;
    }
    if (s->stage != want_from) {
        if (stats) stats->stale_dropped++;
return LOADER_OK;
}

    // cooldown gate: a slot that just failed is parked for a bit.
    if (now_us < s->cooldown_us) {
        return LOADER_RETRY;
    }

    loader_result r = call_hook(hooks, job.kind, cx, cz, &s->c);
switch (r) {
        case LOADER_OK:
            s->stage = loader_job_target_stage(job.kind);
            if (stats) stats->ran[job.kind]++;
            break;
        case LOADER_RETRY:
            // leave the stage where it was; the scheduler will re-enqueue. no
            // cooldown -- retries are "waiting on a neighbour", not a failure.
            if (stats) stats->retried++;
            break;
        case LOADER_FAIL:
            // park the slot. we DONT reset to EMPTY here unless the alloc itself
            // failed -- a gen failure on an alloced chunk can retry without
            // re-allocating. for alloc fails there's no chunk so EMPTY is correct.
            s->cooldown_us = now_us + LOADER_FAIL_COOLDOWN_US;
            if (job.kind == LOADER_JOB_ALLOC) s->stage = LOADER_STAGE_EMPTY;
            if (stats) stats->failed++;
            LOGW("loader: job %s failed at (%d,%d)",
                 loader_job_name(job.kind), cx, cz);
            break;
    }
    return r;
}
