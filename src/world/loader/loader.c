#include "loader.h"
#include <math.h>
#include "../../config.h"
#include "../../util/log.h"
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
typedef struct { loader_evict_fn fn;
void *user;
} free_ctx;
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
// and drop the pointer;
a real integration would hand it to the save system.
static void on_evict(int cx, int cz, chunk *c, void *user) {
    (void)cx; (void)cz; (void)user;
    if (c) chunk_destroy(c);
}

// queue a single job for a slot, scoring it on the way in.
static void schedule(loader *l, loader_slot *s, uint64_t now_us) {
    int next = loader_job_for_stage(s->stage);
if (next < 0) return;
loader_job_kind kind = (loader_job_kind)next;
int prio = loader_priority_score(&l->prio, l->focus, s->cx, s->cz, kind);
uint32_t serial = s->gen;
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
    if (!l->focus.valid) return;
int moved = loader_ring_recenter(&l->ring, l->focus.cx, l->focus.cz,
                                     on_evict, l);
l->ticks_since_scan = 0;
}

    // 3. drain the queue within budget. cpu and upload have separate allowances;
int cpu_left    = l->budget_cpu;
int upload_left = l->budget_upload;
for (int i = 0;
i < LOADER_RING_SLOTS;
}
