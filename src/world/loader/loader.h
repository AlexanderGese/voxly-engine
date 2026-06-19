#ifndef WORLD_LOADER_H
#define WORLD_LOADER_H

#include <stdint.h>
#include "loader_types.h"
#include "loader_queue.h"
#include "loader_ring.h"
#include "loader_priority.h"
#include "loader_worker.h"

// the front door. owns the ring, the queue, the priority config and the stats,
// and exposes the two calls the game loop actually makes each frame:
// loader_tick()  -- update focus, schedule missing work, run a budget of jobs
// loader_visit() -- walk every resident chunk for the renderer
// the engine wires up the hooks once at startup (gen/light/mesh/upload) and then
// never thinks about the pipeline again. that's the goal anyway.

typedef struct {
    loader_ring         ring;
    loader_queue        queue;
    loader_priority_cfg prio;
    loader_hooks        hooks;
    loader_stats        stats;
    loader_focus        focus;

    // per-tick job budget. cpu jobs vs the gl-bound upload job are budgeted
    // separately because an upload stall hurts the frame more than a gen stall.
    int budget_cpu;      // gen/light/mesh/alloc jobs per tick
    int budget_upload;   // upload jobs per tick

    // re-scan throttle. we dont re-walk the whole ring to look for missing work
    // every single frame; only after a recenter or every N ticks.
    int   rescan_interval;
    int   ticks_since_scan;

    uint32_t job_serial_ctr; // monotonic, feeds job tiebreak when slots share gen
} loader;

void loader_init(loader *l, loader_hooks hooks);
void loader_shutdown(loader *l, loader_evict_fn on_free, void *user);

// set the player position (world space) and facing (world space, xz used). cheap,
// call every frame. the tick uses the latest value.
void loader_set_focus(loader *l, vec3 player_pos, vec3 facing);

// the heartbeat. recenters the ring if the player crossed a chunk border,
// (re)schedules jobs for chunks that arent resident, then drains up to the
// per-tick budget from the queue. now_us is a monotonic microsecond clock.
void loader_tick(loader *l, uint64_t now_us);

// resident-chunk iteration for rendering. only chunks at LOADER_STAGE_RESIDENT
// are handed out -- half-built chunks never reach the renderer.
typedef void (*loader_visit_fn)(chunk *c, void *user);
void loader_visit(loader *l, loader_visit_fn fn, void *user);

// queue depth, for the debug overlay.
size_t loader_pending(const loader *l);

#endif
