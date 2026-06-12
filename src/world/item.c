#include "item.h"
#include "world.h"
#include "../math/rng.h"
#include "../config.h"

#include <string.h>
#include <math.h>

static rng irng;

void item_world_init(item_world *iw) {
    memset(iw, 0, sizeof *iw);
    rng_init(&irng, 0xfeedface);
}

static int alloc_item(item_world *iw) {
    for (int i = 0; i < MAX_ITEMS; i++) if (!iw->list[i].alive) return i;
    return -1;
}

void item_spawn(item_world *iw, vec3 pos, block_id id) {
    int k = alloc_item(iw);
    if (k < 0) return;
    dropped_item *d = &iw->list[k];
    d->alive = 1;
    d->block = id;
    d->pos = vec3_add(pos, (vec3){0.5f, 0.5f, 0.5f});
    d->vel = (vec3){
        rng_frange(&irng, -1, 1),
        rng_frange(&irng,  2, 3),
        rng_frange(&irng, -1, 1),
    };
    d->age = 0;
    d->spin = 0;
    iw->count++;
}

void item_update(item_world *iw, world *w, float dt) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        dropped_item *d = &iw->list[i];
        if (!d->alive) continue;
        d->age += dt;
        d->spin += dt * 2.0f;

        d->vel.y += GRAVITY * dt;
        if (d->vel.y < TERMINAL_VELOCITY) d->vel.y = TERMINAL_VELOCITY;

        vec3 np = vec3_add(d->pos, vec3_scale(d->vel, dt));

        // lazy ground check: if block below is solid, zero y vel and snap
        int bx = (int)floorf(np.x);
        int by = (int)floorf(np.y);
        int bz = (int)floorf(np.z);
        block_id under = world_get_block(w, bx, by - 1, bz);
        if (by > 0 && world_get_block(w, bx, by, bz) != BLOCK_AIR) {
            d->vel.y = 0;
            np.y = (float)by + 1.0f;
        } else if (under && under != BLOCK_AIR && d->vel.y < 0) {
            // nothing
        }
        d->pos = np;

        if (d->age > 300.0f) {
            d->alive = 0;
            iw->count--;
        }
    }
}

int item_try_pickup(item_world *iw, vec3 player_pos, block_id *out) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        dropped_item *d = &iw->list[i];
        if (!d->alive) continue;
        if (vec3_distance(d->pos, player_pos) < 1.5f) {
            *out = d->block;
            d->alive = 0;
            iw->count--;
            return 1;
        }
    }
    return 0;
}
