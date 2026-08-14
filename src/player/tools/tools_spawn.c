#include "tools_spawn.h"
#include <math.h>
#include <stddef.h>   // NULL

vec3 tools_spawn_pop_vel(rng *r) {
    // small upward pop, random horizontal jitter. tuned so drops land within a
    // block or so of where they came from.
    float ang = rng_frange(r, 0.0f, 6.2831853f);
    float spd = rng_frange(r, 0.5f, 2.0f);
    vec3 v;
    v.x = spd * 0.15f * cosf(ang);
    v.z = spd * 0.15f * sinf(ang);
    v.y = rng_frange(r, 1.5f, 2.5f);
    return v;
}

// find a free entity slot. mirrors how item_spawn allocates, but we want the
// handle back so we can set a custom velocity.
static dropped_item *alloc_item(item_world *iw) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!iw->list[i].alive) {
            if (i >= iw->count) iw->count = i + 1;
            return &iw->list[i];
        }
    }
    return NULL;   // world full, drop the drop. literally.
}

int tools_spawn_stack(item_world *iw, vec3 block_center, drop_stack stack, rng *r) {
    if (stack.id == BLOCK_AIR || stack.count <= 0) return 0;

    int spawned = 0;
    for (int n = 0; n < stack.count; n++) {
        dropped_item *it = alloc_item(iw);
        if (!it) break;

        it->alive = 1;
        it->block = stack.id;
        // jitter the spawn point a touch so a stack of 4 doesn't z-fight.
        vec3 off = vec3_new(rng_frange(r, -0.15f, 0.15f),
                            rng_frange(r, -0.05f, 0.10f),
                            rng_frange(r, -0.15f, 0.15f));
        it->pos  = vec3_add(block_center, off);
        it->vel  = tools_spawn_pop_vel(r);
        it->age  = 0.0f;
        it->spin = rng_frange(r, 0.0f, 6.2831853f);
        spawned++;
    }
    return spawned;
}

int tools_spawn_drops(item_world *iw, int bx, int by, int bz,
                      const drop_list *drops, rng *r) {
    // center of the block. the +0.5 on each axis puts items mid-voxel.
    vec3 center = vec3_new((float)bx + 0.5f, (float)by + 0.5f, (float)bz + 0.5f);

    int total = 0;
    for (int i = 0; i < drops->count; i++) {
        total += tools_spawn_stack(iw, center, drops->item[i], r);
    }
    return total;
}
