#include "building_drop.h"

#include "../../world/block_ext.h"

vec3 building_drop_pop_velocity(rng *r) {
    // small horizontal scatter, a definite upward kick. numbers tuned by feel:
    // items should bounce up a bit and spread, not rocket away.
    float vx = rng_frange(r, -1.2f, 1.2f);
    float vz = rng_frange(r, -1.2f, 1.2f);
    float vy = rng_frange(r,  2.0f, 3.0f);
    return vec3_new(vx * 0.5f, vy, vz * 0.5f);
}

building_drop_spawn building_drop_one(block_id id, int count, vec3 pos, rng *r) {
    building_drop_spawn s;
    s.id = id;
    s.count = count;
    // nudge off the exact center so two drops from one break don't stack on the
    // same pixel before physics separates them.
    vec3 jitter = vec3_new(rng_frange(r, -0.1f, 0.1f),
                           rng_frange(r, -0.05f, 0.05f),
                           rng_frange(r, -0.1f, 0.1f));
    s.pos = vec3_add(pos, jitter);
    s.vel = building_drop_pop_velocity(r);
    return s;
}

int building_drop_split(const building_drop *d, building_drop_spawn *out,
                        int max_out, rng *r) {
    if (!d || !out || max_out <= 0) return 0;

    int total = d->drop_count;
    if (total <= 0) return 0;

    // respect the block's stack cap so a fat drop fragments into real stacks.
    const block_ext_info *bi = block_ext_get(d->drop_id);
    int cap = (bi && bi->stack_max > 0) ? bi->stack_max : 64;

    int written = 0;
    while (total > 0 && written < max_out) {
        int n = total > cap ? cap : total;
        out[written] = building_drop_one(d->drop_id, n, d->drop_pos, r);
        total -= n;
        written++;
    }

    // if we capped out with stuff left over, fold the remainder into the last
    // descriptor's count. better an oversized stack than vanished items.
    if (total > 0 && written > 0) {
        out[written - 1].count += total;
    }
    return written;
}
