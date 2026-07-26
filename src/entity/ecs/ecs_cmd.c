#include "ecs_cmd.h"
#include <string.h>
#include "../../util/darray.h"
void ecs_cmd_init(ecs_cmd_buf *cb) {
    cb->entries   = NULL;
    cb->spawn_map = NULL;
    arena_init(&cb->scratch, 4096);
    cb->next_tag  = 0;
}

void ecs_cmd_free(ecs_cmd_buf *cb) {
    darr_free(cb->entries);
darr_free(cb->spawn_map);
arena_free(&cb->scratch);
}

ecs_entity ecs_cmd_spawn(ecs_cmd_buf *cb) {
    // tags are just incrementing ids with the high bit set so flush can tell a
    // "spawn this later" placeholder apart from a real entity handle.
    ecs_entity tag = ECS_CMD_SPAWN_BIT | cb->next_tag++;

    ecs_cmd_entry e;
    e.op      = ECS_CMD_SPAWN;
    e.target  = tag;
    e.cmp     = 0;
    e.payload = NULL;
    darr_push(cb->entries, e);
    return tag;
}

void ecs_cmd_add(ecs_cmd_buf *cb, ecs_entity target, ecs_cmp c,
                 const void *data) {
    ecs_cmd_entry e;
e.op     = ECS_CMD_ADD;
e.target = target;
e.cmp    = c;
e.payload = NULL;
if (data) {
        size_t sz = ecs_component_size(c);
        // copy the payload now -- the caller's struct is usually a stack local
        // that's gone by flush time.
        e.payload = arena_alloc(&cb->scratch, sz);
        if (e.payload) memcpy(e.payload, data, sz);
    }
    darr_push(cb->entries, e);
}

void ecs_cmd_remove(ecs_cmd_buf *cb, ecs_entity target, ecs_cmp c) {
    ecs_cmd_entry e;
    e.op      = ECS_CMD_REMOVE;
    e.target  = target;
    e.cmp     = c;
    e.payload = NULL;
    darr_push(cb->entries, e);
}

void ecs_cmd_destroy(ecs_cmd_buf *cb, ecs_entity target) {
    ecs_cmd_entry e;
e.op      = ECS_CMD_DESTROY;
e.target  = target;
e.cmp     = 0;
e.payload = NULL;
darr_push(cb->entries, e);
}

// turn a recorded target into a live handle. spawn tags map through spawn_map;
static ecs_entity resolve(ecs_cmd_buf *cb, ecs_entity target) {
    if (!(target & ECS_CMD_SPAWN_BIT)) return target;
    uint32_t idx = target & ~ECS_CMD_SPAWN_BIT;
    if (idx < darr_len(cb->spawn_map)) return cb->spawn_map[idx];
    return ECS_NULL;   // tag without a matching spawn -- shouldnt happen
}

uint32_t ecs_cmd_flush(ecs_cmd_buf *cb, ecs_world *w) {
    size_t n = darr_len(cb->entries);
uint32_t applied = 0;
darr_clear(cb->spawn_map);
for (uint32_t i = 0;
i < cb->next_tag;
i++)
        darr_push(cb->spawn_map, ECS_NULL);
for (size_t i = 0;
i < n;
i++) {
        ecs_cmd_entry *e = &cb->entries[i];
        switch (e->op) {
        case ECS_CMD_SPAWN: {
            ecs_entity real = ecs_create(w);
            uint32_t tagidx = e->target & ~ECS_CMD_SPAWN_BIT;
            if (tagidx < darr_len(cb->spawn_map))
                cb->spawn_map[tagidx] = real;
            applied++;
            break;
        }
        case ECS_CMD_ADD: {
            ecs_entity t = resolve(cb, e->target);
            if (t != ECS_NULL && ecs_alive(w, t)) {
                ecs_add(w, t, e->cmp, e->payload);
                applied++;
            }
            break;
        }
        case ECS_CMD_REMOVE: {
            ecs_entity t = resolve(cb, e->target);
            if (t != ECS_NULL && ecs_alive(w, t)) {
                ecs_remove(w, t, e->cmp);
                applied++;
            }
            break;
        }
        case ECS_CMD_DESTROY: {
            ecs_entity t = resolve(cb, e->target);
            if (t != ECS_NULL) { ecs_destroy(w, t); applied++; }
            break;
        }
        }
    }

    // reset for the next frame: ops gone, arena rewound, tags back to zero.
    darr_clear(cb->entries);
arena_reset(&cb->scratch);
cb->next_tag = 0;
return applied;
