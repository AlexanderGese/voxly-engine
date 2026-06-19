#ifndef WORLD_LOADER_HOOKS_WORLD_H
#define WORLD_LOADER_HOOKS_WORLD_H
#include "loader_worker.h"
#include "../world.h"
// the concrete hookup between the loader pipeline and the actual engine. this is
// the one file in loader/ that's allowed to know about worldgen / lighting / the
typedef struct {
    world   *w;
    unsigned seed;
} loader_world_ctx;
loader_hooks loader_hooks_world(loader_world_ctx *ctx);
#endif
