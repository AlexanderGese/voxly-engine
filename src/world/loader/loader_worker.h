#ifndef WORLD_LOADER_WORKER_H
#define WORLD_LOADER_WORKER_H
#include <stdint.h>
#include "loader_types.h"
#include "loader_job.h"
#include "loader_ring.h"
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
typedef struct {
    uint64_t ran[5];     // jobs completed, indexed by loader_job_kind
    uint64_t retried;
    uint64_t failed;
    uint64_t stale_dropped;
} loader_stats;
#define LOADER_FAIL_COOLDOWN_US  (250 * 1000)   // 250ms
#endif
