#include "mspawn_despawn.h"
#include "../../math/vec3.h"
#include <string.h>

// how long a mob may linger in the soft band before it leaves regardless of
// the dice. keeps a mob from camping right at 49 blocks forever just because
// the rng kept smiling on it.
#define MSPAWN_LINGER_MAX 30.0f

void mspawn_despawn_init(mspawn_despawn_state *st) {
    memset(st, 0, sizeof *st);
}

int mspawn_despawn_tick(mspawn_despawn_state *st, mob_registry *mr,
                        vec3 player_pos, mspawn_rng *r, float dt) {
    int culled = 0;

    for (int i = 0; i < MAX_MOBS; i++) {
        entity *e = &mr->list[i];
        if (!e->alive) { st->linger[i] = 0.0f; continue; }

        float d = vec3_distance(e->pos, player_pos);

        if (d > MSPAWN_DESPAWN_HARD) {
            // past the hard radius: gone, no roll.
            mob_remove(mr, e->id);
            st->linger[i] = 0.0f;
            culled++;
            continue;
        }

        if (d > MSPAWN_DESPAWN_SOFT) {
            // soft band: accumulate time and roll a slow leave chance. the
            // longer it sits out here the more certain its exit becomes.
            st->linger[i] += dt;
            int leave = mspawn_rng_chance(r, MSPAWN_DESPAWN_CHANCE);
            if (st->linger[i] >= MSPAWN_LINGER_MAX) leave = 1;
            if (leave) {
                mob_remove(mr, e->id);
                st->linger[i] = 0.0f;
                culled++;
            }
        } else {
            // back inside the comfortable radius, reset the clock.
            st->linger[i] = 0.0f;
        }
    }

    return culled;
}
