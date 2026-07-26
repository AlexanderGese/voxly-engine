#ifndef ENTITY_ECS_SNAPSHOT_H
#define ENTITY_ECS_SNAPSHOT_H

#include <stdint.h>
#include <stddef.h>

#include "ecs_world.h"

// dump / restore the ecs to a flat byte blob so entities survive a save/load.
// format is dead simple and version-gated off SAVE_VERSION: a header, then for
// each live entity its handle + signature + every component blob it owns, in
// component-id order. no compression, the world save code wraps us and zips the
// whole thing anyway.
//
// we deliberately re-create handles with the same index/generation on load so
// cross-entity references (ai.target, etc) stay valid without a fixup pass.

#define ECS_SNAP_MAGIC   0x45435330u   /* 'ECS0' */

typedef struct {
    uint8_t *data;
    size_t   len;
    size_t   cap;
} ecs_blob;

void ecs_blob_init(ecs_blob *b);
void ecs_blob_free(ecs_blob *b);

// serialize every live entity into `out` (which is reset first). returns the
// number of entities written.
uint32_t ecs_snapshot_save(ecs_world *w, ecs_blob *out);

// rebuild `w` from a blob. `w` must already be ecs_world_init'd; it gets cleared
// first. returns 1 on success, 0 on a bad/short/version-mismatched blob.
int ecs_snapshot_load(ecs_world *w, const uint8_t *data, size_t len);

#endif
