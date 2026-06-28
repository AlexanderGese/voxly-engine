#ifndef WORLD_STRUCTGEN_H
#define WORLD_STRUCTGEN_H
#include "structgen_types.h"
#include "structgen_buffer.h"
#include "../chunk.h"
typedef int (*structgen_height_fn)(int wx, int wz, void *user);
typedef struct {
    structgen_config    cfg;
    uint32_t            world_seed;
    structgen_height_fn height;     // may be NULL -> falls back to sea level
    void               *height_user;
    structgen_buffer    scratch;    // reused across calls to avoid churn
} structgen_ctx;
void structgen_ctx_init(structgen_ctx *ctx, uint32_t world_seed,
                        structgen_height_fn height, void *height_user);
#endif
