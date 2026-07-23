#include "mob.h"
#include "ai.h"

#include <string.h>

void mob_registry_init(mob_registry *mr) {
    memset(mr, 0, sizeof *mr);
}

int mob_spawn(mob_registry *mr, entity_type t, vec3 pos) {
    for (int i = 0; i < MAX_MOBS; i++) {
        if (!mr->list[i].alive) {
            mr->list[i] = entity_new(t, pos);
            mr->count++;
            return (int)mr->list[i].id;
        }
    }
    return -1;
}

void mob_remove(mob_registry *mr, uint32_t id) {
    for (int i = 0; i < MAX_MOBS; i++) {
        if (mr->list[i].id == id) {
            mr->list[i].alive = 0;
            mr->count--;
            return;
        }
    }
}

void mob_update_all(mob_registry *mr, world *w, const player *p, float dt) {
    for (int i = 0; i < MAX_MOBS; i++) {
        entity *e = &mr->list[i];
        if (!e->alive) continue;
        ai_tick(e, p, w, dt);
        if (e->hp <= 0) {
            e->alive = 0;
            mr->count--;
        }
    }
}

entity *mob_by_id(mob_registry *mr, uint32_t id) {
    for (int i = 0; i < MAX_MOBS; i++) {
        if (mr->list[i].alive && mr->list[i].id == id) return &mr->list[i];
    }
    return NULL;
}
