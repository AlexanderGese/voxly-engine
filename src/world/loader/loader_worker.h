#ifndef WORLD_LOADER_WORKER_H
#define WORLD_LOADER_WORKER_H

#include <stdint.h>
#include "loader_types.h"
#include "loader_job.h"
#include "loader_ring.h"

// the worker executes jobs. it's deliberately ignorant of WHERE the work comes
// from -- the actual gen/light/mesh code lives elsewhere in the engine, we just
// call through a vtable of hooks. that keeps loader/ decoupled from worldgen.c /
// lighting.c / the mesher, and lets the tests stub them out with fakes.

// the hooks. each returns a loader_result. they operate on a single chunk that
// the worker has already pulled out of the ring. NULL hooks are treated as a
// no-op success (handy for bringing the pipeline up stage by stage).
typedef struct {
    // make the chunk object. on OK the chunk MUST be written back via out_chunk.
    loader_result (*alloc)(int cx, int cz, chunk **out_chunk, void *user);
    // fill terrain. chunk is non-NULL and ALLOCED.
    loader_result (*gen)(chunk *c, void *user);
    // propagate light. may RETRY if a neighbour isnt generated yet.
    loader_result (*light)(chunk *c, void *user);
    // build the cpu-side mesh. may RETRY if neighbours arent lit (seam culling).
    loader_result (*mesh)(chunk *c, void *user);
    // push the mesh to the gpu. main thread only. should not RETRY in practice.
    loader_result (*upload)(chunk *c, void *user);
    void *user;
} loader_hooks;

// counters so the debug overlay can show what the loader is chewing on.
typedef struct {
    uint64_t ran[5];     // jobs completed, indexed by loader_job_kind
    uint64_t retried;
    uint64_t failed;
    uint64_t stale_dropped;
} loader_stats;

// run a single job against the ring. resolves the target slot, validates the
// job isnt stale (gen mismatch) or out of range, calls the right hook, and
// advances the slot's stage on success. returns the result, or LOADER_OK with
// stale_dropped++ if the job was discarded. `now_us` feeds cooldowns.
loader_result loader_worker_run(loader_ring *ring, const loader_hooks *hooks,
                                loader_job job, loader_stats *stats,
                                uint64_t now_us);

// how long a slot sits in cooldown after a FAIL before it can be re-queued. keeps
// a chunk that legitimately cant generate (e.g. oom) from pegging a cpu.
#define LOADER_FAIL_COOLDOWN_US  (250 * 1000)   // 250ms

#endif
