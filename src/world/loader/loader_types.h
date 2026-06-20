#ifndef WORLD_LOADER_TYPES_H
#define WORLD_LOADER_TYPES_H

#include <stdint.h>
#include "../chunk.h"
#include "../../math/vec3.h"

// shared plain-data for the async chunk loader. the loader sits between the
// world (which owns chunks) and the worldgen/mesher (which do the heavy work).
// the idea: every frame we figure out which chunks SHOULD be resident around
// the player, hand the missing ones to a job queue, and pump that queue a few
// jobs at a time so we never stall the frame. nothing here spins up an actual
// OS thread yet -- the "async" is cooperative, we slice the work. when i bolt
// pthreads on later the only thing that changes is who calls loader_worker_run.

// a chunk goes through these stages on its way to being drawable. order matters,
// loader_stage_next() walks them in sequence. a chunk that fails any stage gets
// kicked back to EMPTY and re-queued (with a cooldown so we dont thrash).
typedef enum {
    LOADER_STAGE_EMPTY    = 0,   // no chunk object yet
    LOADER_STAGE_ALLOCED  = 1,   // chunk struct exists, blocks not filled
    LOADER_STAGE_GENERATED= 2,   // terrain noise written
    LOADER_STAGE_LIT      = 3,   // sunlight/blocklight propagated
    LOADER_STAGE_MESHED   = 4,   // vertex buffer built, ready to draw
    LOADER_STAGE_RESIDENT = 5    // fully live, nothing left to do
} loader_stage;

// what a single job actually does. one job advances a chunk by exactly one
// stage. keeping them one-step makes the queue easy to re-prioritise mid-flight
// -- a chunk that drifts out of range stops getting new jobs and just sits.
typedef enum {
    LOADER_JOB_ALLOC = 0,   // EMPTY    -> ALLOCED
    LOADER_JOB_GEN   = 1,   // ALLOCED  -> GENERATED
    LOADER_JOB_LIGHT = 2,   // GENERATED-> LIT
    LOADER_JOB_MESH  = 3,   // LIT      -> MESHED
    LOADER_JOB_UPLOAD= 4    // MESHED   -> RESIDENT (gl upload, main thread only)
} loader_job_kind;

// result of running a job. RETRY means "couldnt finish, leave the chunk where it
// was and try again later" -- usually a missing neighbour for the mesher.
typedef enum {
    LOADER_OK    = 0,
    LOADER_RETRY = 1,
    LOADER_FAIL  = 2
} loader_result;

// packs a chunk coord into a u64 the same way the world hashmap does. we keep a
// private copy of the helper so loader/ doesnt have to drag in hashmap.h just
// for one inline.
static inline uint64_t loader_key(int cx, int cz) {
    return ((uint64_t)(uint32_t)cx) | (((uint64_t)(uint32_t)cz) << 32);
}

static inline void loader_unkey(uint64_t k, int *cx, int *cz) {
    *cx = (int)(int32_t)(k & 0xffffffffu);
    *cz = (int)(int32_t)(k >> 32);
}

// where the player is, in chunk coords, plus a heading so we can bias loading
// toward where theyre looking. heading is a unit-ish vec in world space, we only
// use its xz. filled by loader_set_focus once per frame.
typedef struct {
    int   cx, cz;       // player chunk
    float fx, fz;       // facing direction, xz, not normalised strictly
    int   valid;
} loader_focus;

#endif
