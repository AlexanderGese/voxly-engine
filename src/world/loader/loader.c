#include "loader.h"

#include <math.h>
#include "../../config.h"
#include "../../util/log.h"

// default budgets. tuned by feel: ~16 cpu jobs/tick keeps the world filling in
// without blowing the frame, 4 uploads keeps gl driver churn bounded. the rescan
// every 4 ticks is plenty -- the player cant outrun the ring in 4 frames.
#define LOADER_DEFAULT_BUDGET_CPU     16
#define LOADER_DEFAULT_BUDGET_UPLOAD   4
#define LOADER_DEFAULT_RESCAN          4

void loader_init(loader *l, loader_hooks hooks) {
    loader_ring_init(&l->ring);
    loader_queue_init(&l->queue, 256);
    l->prio  = loader_priority_default();
    l->hooks = hooks;
    l->focus = (loader_focus){0, 0, 0.0f, 1.0f, 0};

    l->stats = (loader_stats){0};

    l->budget_cpu     = LOADER_DEFAULT_BUDGET_CPU;
    l->budget_upload  = LOADER_DEFAULT_BUDGET_UPLOAD;
    l->rescan_interval= LOADER_DEFAULT_RESCAN;
    l->ticks_since_scan = LOADER_DEFAULT_RESCAN;   // force a scan on first tick
    l->job_serial_ctr = 0;
}

// adaptor: the ring frees chunks via a (chunk*, void*) callback, the public API
// hands the engine a (cx,cz,chunk*, void*) eviction callback. we bounce through a
// little context so shutdown can reuse the recenter-style hook.
typedef struct { loader_evict_fn fn; void *user; } free_ctx;

static void free_bounce(chunk *c, void *u) {
    free_ctx *fc = u;
    if (fc->fn && c) fc->fn(c->cx, c->cz, c, fc->user);
}

void loader_shutdown(loader *l, loader_evict_fn on_free, void *user) {
    free_ctx fc = { on_free, user };
    loader_ring_free(&l->ring, free_bounce, &fc);
    loader_queue_free(&l->queue);
}

void loader_set_focus(loader *l, vec3 player_pos, vec3 facing) {
    int cx, cz;
    // world -> chunk. floor division because negatives.
    cx = (int)floorf(player_pos.x / (float)CHUNK_SIZE_X);
    cz = (int)floorf(player_pos.z / (float)CHUNK_SIZE_Z);
    l->focus.cx = cx;
    l->focus.cz = cz;
    l->focus.fx = facing.x;
    l->focus.fz = facing.z;
    l->focus.valid = 1;
}

// when a chunk leaves the ring its mesh/light state is the loader's problem, but
// the chunk object itself belongs to whoever wired the hooks. for now we just log
// and drop the pointer; a real integration would hand it to the save system.
static void on_evict(int cx, int cz, chunk *c, void *user) {
    (void)cx; (void)cz; (void)user;
    if (c) chunk_destroy(c);
}

// queue a single job for a slot, scoring it on the way in.
static void schedule(loader *l, loader_slot *s, uint64_t now_us) {
    int next = loader_job_for_stage(s->stage);
    if (next < 0) return;   // resident, nothing to do
    loader_job_kind kind = (loader_job_kind)next;

    int prio = loader_priority_score(&l->prio, l->focus, s->cx, s->cz, kind);
    uint32_t serial = s->gen;          // ABA guard
    // fold a per-loader counter into the low bits so two slots with the same gen
    // still get a stable tiebreak instead of fighting over enqueue time alone.
    (void)l->job_serial_ctr;
    loader_job j = loader_job_make(loader_key(s->cx, s->cz), kind,
                                   prio, serial, now_us);
    loader_queue_push(&l->queue, j);
}

// walk the ring and enqueue work for every non-resident, non-cooldown slot that
// doesnt already have its next stage in flight. we DONT dedupe perfectly -- the
// worker's stage check drops duplicates cheaply, so a stray double-enqueue is
// harmless and not worth a per-slot "queued" flag.
static void rescan(loader *l, uint64_t now_us) {
    for (int i = 0; i < LOADER_RING_SLOTS; i++) {
        loader_slot *s = &l->ring.slots[i];
        if (!s->occupied) continue;
        if (s->stage == LOADER_STAGE_RESIDENT) continue;
        if (now_us < s->cooldown_us) continue;
        schedule(l, s, now_us);
    }
}

void loader_tick(loader *l, uint64_t now_us) {
    if (!l->focus.valid) return;   // dont do anything until we know where we are

    // 1. recenter. if the player crossed a chunk border, slots roll over and any
    // job targeting an evicted slot is invalidated by its bumped gen.
    int moved = loader_ring_recenter(&l->ring, l->focus.cx, l->focus.cz,
                                     on_evict, l);
    if (moved > 0) {
        // a recenter can leave stale jobs pointing at vacated indices. theyre
        // caught at pop time by the gen check, but force a fresh scan so newly
        // exposed slots get work this tick instead of waiting for the interval.
        l->ticks_since_scan = l->rescan_interval;
    }

    // 2. (re)schedule missing work, throttled.
    if (++l->ticks_since_scan >= l->rescan_interval) {
        rescan(l, now_us);
        l->ticks_since_scan = 0;
    }

    // 3. drain the queue within budget. cpu and upload have separate allowances;
    // a retried job is re-scheduled (not re-run) so we dont spin on a blocked
    // neighbour within a single tick.
    int cpu_left    = l->budget_cpu;
    int upload_left = l->budget_upload;

    while ((cpu_left > 0 || upload_left > 0) && !loader_queue_empty(&l->queue)) {
        loader_job peek;
        loader_queue_peek(&l->queue, &peek);

        int is_upload = loader_job_is_main_thread(peek.kind);
        if (is_upload && upload_left <= 0) {
            // most-urgent job is an upload but we're out of upload budget. since
            // the heap is priority-ordered we cant cheaply skip past it, so we
            // stop draining this tick. uploads are rare enough that this is fine.
            break;
        }
        if (!is_upload && cpu_left <= 0) {
            break;   // same story, out of cpu budget and the top is a cpu job
        }

        loader_job job;
        loader_queue_pop(&l->queue, &job);

        loader_result r = loader_worker_run(&l->ring, &l->hooks, job,
                                            &l->stats, now_us);
        if (is_upload) upload_left--; else cpu_left--;

        if (r == LOADER_RETRY) {
            // re-home the job: bump nothing, just push it back with a fresh score.
            // it'll wait behind anything now more urgent (likely the neighbour it
            // was blocked on). guard against re-running it this same tick by not
            // looping on the same slot -- the budget decrement above handles that.
            loader_slot *s = loader_ring_slot_for(&l->ring,
                (int)(int32_t)(job.key & 0xffffffffu),
                (int)(int32_t)(job.key >> 32));
            if (s && s->stage != LOADER_STAGE_RESIDENT && now_us >= s->cooldown_us)
                schedule(l, s, now_us);
        }
    }
}

void loader_visit(loader *l, loader_visit_fn fn, void *user) {
    if (!fn) return;
    for (int i = 0; i < LOADER_RING_SLOTS; i++) {
        loader_slot *s = &l->ring.slots[i];
        if (!s->occupied) continue;
        if (s->stage != LOADER_STAGE_RESIDENT) continue;
        if (!s->c) continue;   // shouldnt happen at RESIDENT, but be safe
        fn(s->c, user);
    }
}

size_t loader_pending(const loader *l) {
    return loader_queue_len(&l->queue);
}
