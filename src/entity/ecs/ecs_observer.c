#include "ecs_observer.h"

void ecs_observers_init(ecs_observers *o) {
    o->count = 0;
    for (int i = 0; i < ECS_MAX_OBSERVERS; i++)
        o->obs[i].active = 0;
}

int ecs_observe(ecs_observers *o, ecs_cmp c, ecs_hook_kind kind,
                ecs_observer_fn fn, void *user) {
    // reuse a hole left by ecs_unobserve before growing the high water mark.
    int slot = -1;
    for (int i = 0; i < o->count; i++) {
        if (!o->obs[i].active) { slot = i; break; }
    }
    if (slot < 0) {
        if (o->count >= ECS_MAX_OBSERVERS) return -1;
        slot = o->count++;
    }
    o->obs[slot].fn     = fn;
    o->obs[slot].user   = user;
    o->obs[slot].cmp    = c;
    o->obs[slot].kind   = kind;
    o->obs[slot].active = 1;
    return slot;
}

void ecs_unobserve(ecs_observers *o, int handle) {
    if (handle < 0 || handle >= o->count) return;
    o->obs[handle].active = 0;
    o->obs[handle].fn     = NULL;
}

void ecs_obs_fire(ecs_world *w, ecs_observers *o, ecs_entity e, ecs_cmp c,
                  ecs_hook_kind kind) {
    if (!o) return;
    // grab the slot once so every observer sees the same pointer. for a remove
    // hook the component is still physically in the store at fire time -- the
    // wrapper deliberately fires *before* it pulls the component out.
    void *slot = ecs_get(w, e, c);
    for (int i = 0; i < o->count; i++) {
        ecs_observer *ob = &o->obs[i];
        if (!ob->active)          continue;
        if (ob->cmp != c)         continue;
        if (ob->kind != kind)     continue;
        ob->fn(w, e, c, slot, ob->user);
    }
}

void *ecs_obs_add(ecs_world *w, ecs_observers *o, ecs_entity e, ecs_cmp c,
                  const void *data) {
    int existed = ecs_has(w, e, c);
    void *slot  = ecs_add(w, e, c, data);
    // only fire ADD when the component is genuinely new. re-adding to overwrite
    // data isnt a lifecycle event, the thing was already there.
    if (slot && !existed)
        ecs_obs_fire(w, o, e, c, ECS_HOOK_ADD);
    return slot;
}

void ecs_obs_remove(ecs_world *w, ecs_observers *o, ecs_entity e, ecs_cmp c) {
    if (!ecs_has(w, e, c)) return;
    // fire while the data is still attached so the hook can read it one last
    // time (free a gl handle stashed in the component, etc), *then* detach.
    ecs_obs_fire(w, o, e, c, ECS_HOOK_REMOVE);
    ecs_remove(w, e, c);
}
