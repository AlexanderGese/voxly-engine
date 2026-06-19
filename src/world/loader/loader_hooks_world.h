#ifndef WORLD_LOADER_HOOKS_WORLD_H
#define WORLD_LOADER_HOOKS_WORLD_H

#include "loader_worker.h"
#include "../world.h"

// the concrete hookup between the loader pipeline and the actual engine. this is
// the one file in loader/ that's allowed to know about worldgen / lighting / the
// mesher -- everything else stays abstract behind loader_hooks. swap this out and
// the loader runs against fakes for tests.

// context the hooks need: the world (chunk owner + neighbour lookup) and the
// seed. lives wherever the caller keeps it, we just hold a pointer.
typedef struct {
    world   *w;
    unsigned seed;
} loader_world_ctx;

// build a loader_hooks bound to this context. the returned struct's `user` points
// at `ctx`, so `ctx` must outlive the loader. (it's the world, it always does.)
loader_hooks loader_hooks_world(loader_world_ctx *ctx);

#endif
