#include "spawnx_persist.h"
#include <string.h>
#include <math.h>

void spawnx_despawn_init(spawnx_despawn_state *st) {
    memset(st, 0, sizeof *st);
}

static float dist_xz(vec3 a, vec3 b) {
    float dx = a.x - b.x, dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

int spawnx_despawn_tick(spawnx_despawn_state *st, mob_registry *mr,
                        const spawnx_anchor_store *anchors,
                        vec3 player_pos, mspawn_rng *r, float dt) {
    int culled = 0;

    // walk all slots, not mr->count: dead slots leave holes the count doesnt
    // describe, and linger[] is indexed by slot so we want the slot anyway.
    for (int i = 0; i < MAX_MOBS; i++) {
        entity *e = &mr->list[i];
        if (!e->alive) { st->linger[i] = 0.0f; continue; }

        // held mobs never despawn, full stop. clear their linger so if the
        // anchor ever lapses they start the soft clock fresh.
        if (spawnx_anchor_is_held(anchors, e->id)) {
            st->linger[i] = 0.0f;
            continue;
        }

        float d = dist_xz(e->pos, player_pos);

        if (d >= SPAWNX_DESPAWN_HARD) {
            // gone immediately past the hard radius.
            mob_remove(mr, e->id);
            st->linger[i] = 0.0f;
            culled++;
            continue;
        }

        if (d >= SPAWNX_DESPAWN_SOFT) {
            // in the soft band: accrue linger and roll the slow leave chance.
            // scale the chance by dt so the per-tick rate is framerate-stable.
            st->linger[i] += dt;
            float p = SPAWNX_DESPAWN_CHANCE * dt * 20.0f;  // tuned at ~20hz
            // a mob that has lingered a long while leaves with rising odds, so
            // edge-huggers eventually clear instead of re-rolling forever.
            if (st->linger[i] > 30.0f) p += 0.05f;
            if (mspawn_rng_chance(r, p)) {
                mob_remove(mr, e->id);
                st->linger[i] = 0.0f;
                culled++;
            }
        } else {
            // close enough; reset the soft clock.
            st->linger[i] = 0.0f;
        }
    }

    return culled;
}
