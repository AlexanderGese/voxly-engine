#ifndef ENTITY_ECS_CMD_H
#define ENTITY_ECS_CMD_H

#include <stdint.h>

#include "ecs_world.h"
#include "ecs_components.h"
#include "../../util/arena.h"

// deferred command buffer. structural changes mid-iteration are the classic ecs
// footgun: a system walking a query that adds a component (and so resizes the
// store it's iterating) can shuffle the dense array out from under its own
// cursor. ecs_destroy already defers via the world's queue, but add/remove dont,
// and sometimes you want to *spawn* a fresh entity from inside a tick too.
//
// so a system records its structural edits into one of these and the tick driver
// replays them after the iteration finishes. component payloads get copied into
// a bump arena owned by the buffer so the caller's locals can go out of scope
// before the replay. all very boring and exactly what you want.

typedef enum {
    ECS_CMD_SPAWN = 0,   // create a new entity; later cmds can target it by tag
    ECS_CMD_ADD,         // add a component (payload copied into the arena)
    ECS_CMD_REMOVE,      // strip a component
    ECS_CMD_DESTROY      // kill an entity
} ecs_cmd_op;

typedef struct {
    ecs_cmd_op op;
    ecs_entity target;   // entity to edit, or a spawn-tag for ECS_CMD_SPAWN
    ecs_cmp    cmp;      // component id, for ADD / REMOVE
    void      *payload;  // arena ptr to copied component data, ADD only
} ecs_cmd_entry;

typedef struct {
    ecs_cmd_entry *entries;     // darray of recorded ops
    arena          scratch;     // payload storage, reset on flush
    ecs_entity    *spawn_map;   // darray: spawn-tag -> resolved real entity
    uint32_t       next_tag;    // hands out spawn tags (high bit set so they
                                // never collide with a real entity index)
} ecs_cmd_buf;

#define ECS_CMD_SPAWN_BIT  0x80000000u

void ecs_cmd_init(ecs_cmd_buf *cb);
void ecs_cmd_free(ecs_cmd_buf *cb);

// reserve a spawn slot, returning a tag you can hand to ecs_cmd_add as a target
// before the real entity exists. resolved to a real handle at flush time.
ecs_entity ecs_cmd_spawn(ecs_cmd_buf *cb);

// queue an add. `data` (size = ecs_component_size(c)) is copied immediately into
// the buffer's arena; the source can die before flush.
void ecs_cmd_add(ecs_cmd_buf *cb, ecs_entity target, ecs_cmp c, const void *data);
void ecs_cmd_remove(ecs_cmd_buf *cb, ecs_entity target, ecs_cmp c);
void ecs_cmd_destroy(ecs_cmd_buf *cb, ecs_entity target);

// replay every queued op against the world in record order, resolving spawn
// tags as it goes, then reset the buffer. returns the number of ops applied.
uint32_t ecs_cmd_flush(ecs_cmd_buf *cb, ecs_world *w);

uint32_t ecs_cmd_pending(const ecs_cmd_buf *cb);

#endif
