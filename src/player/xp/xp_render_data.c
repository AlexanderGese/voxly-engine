#include "xp_render_data.h"

#include <math.h>
#include <stddef.h>

#include "xp_config.h"
#include "xp_orb.h"
#include "xp_tier.h"

size_t xp_render_instance_stride(void) {
    return sizeof(xp_orb_instance);
}

int xp_render_build(xp_orb_pool *pool, xp_orb_instance *out, int cap, float t) {
    if (!out || cap <= 0) return 0;

    int n = 0;
    int slots = xp_orb_capacity(pool);

    for (int i = 0; i < slots && n < cap; i++) {
        xp_orb *o = xp_orb_at(pool, i);
        if (!o || !o->alive) continue;

        const xp_tier_info *ti = xp_tier_get(o->tier);

        // cosmetic vertical bob, phase-offset per orb so a cluster shimmers.
        float bob = sinf(t * 2.2f + o->bob_phase) * 0.06f;

        // a freshly-spawned orb fades in over its first ~0.25s, and an orb in
        // its last second of life fades out. otherwise fully opaque.
        float alpha = 1.0f;
        if (o->age < 0.25f) {
            alpha = o->age / 0.25f;
        } else if (o->age > XP_ORB_LIFETIME - 1.0f) {
            float left = XP_ORB_LIFETIME - o->age;
            if (left < 0.0f) left = 0.0f;
            alpha = left;
        }

        // magnetized orbs pulse a touch brighter/bigger as they home in — a
        // little juice so pickups feel good.
        float pulse = o->magnetized ? 1.0f + 0.15f * sinf(t * 18.0f + o->bob_phase) : 1.0f;

        xp_orb_instance *inst = &out[n++];
        inst->pos = vec3_new(o->pos.x, o->pos.y + bob, o->pos.z);
        inst->scale = ti->radius * pulse;
        inst->color = xp_tier_color(o->tier);
        inst->alpha = alpha;
    }

    return n;
}
