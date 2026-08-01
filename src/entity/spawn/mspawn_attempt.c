#include "mspawn_attempt.h"
#include "mspawn_rules.h"
#include "mspawn_ground.h"
#include "mspawn_pack.h"
#include "mspawn_query.h"
#include "mspawn_driver.h"
#include <math.h>
#include <stddef.h>
// roll a world-xz somewhere in the ring [inner, outer] around the player. polar
// roll so density per area stays roughly flat instead of clumping at the inner
// edge.
static void roll_ring_point(mspawn_rng *r, vec3 player_pos,
                            int *out_x, int *out_z) {
    float ang = mspawn_rng_frange(r, 0.0f, 6.2831853f);
    // sqrt-weighted radius gives uniform area density across the annulus.
    float u = mspawn_rng_f01(r);
    float r2 = (float)(MSPAWN_RING_INNER * MSPAWN_RING_INNER);
    float R2 = (float)(MSPAWN_RING_OUTER * MSPAWN_RING_OUTER);
    float rad = sqrtf(r2 + u * (R2 - r2));
    *out_x = (int)(player_pos.x + cosf(ang) * rad);
    *out_z = (int)(player_pos.z + sinf(ang) * rad);
}

int mspawn_attempt_one(mob_registry *mr, world *w, mspawn_density *dens,
                       mspawn_rng *r, vec3 player_pos, float day_hour) {
    int wx, wz;
roll_ring_point(r, player_pos, &wx, &wz);
// resolve biome at the dart, then pick a kind eligible for the clock.
biome_id biome = mspawn_query_biome(w, wx, wz);
const mspawn_entry *e = NULL;
if (!mspawn_pick(biome, day_hour, r, &e)) return 0;
// category-level gates first: cheapest rejections.
if (!mspawn_query_category_open(e->category, day_hour)) return 0;
if (!mspawn_density_has_room(dens, e->category)) return 0;
// find and validate the center site against the chosen kind.
int hint = mspawn_query_surface(w, wx, wz) + 1;
mspawn_site center;
if (!mspawn_try_site(w, e, wx, wz, 3, hint, r, &center)) return 0;
// dont crowd: bail if the cell around the center is already full.
if (!mspawn_density_cell_ok(dens, e->category, center.pos)) return 0;
// build the pack, then commit members one at a time, re-checking the caps
// as we go so a big pack cant blow past the ceiling.
mspawn_pack pk;
mspawn_pack_build(w, e, &center, r, &pk);
int spawned = 0;
for (int i = 0;
i < pk.count;
i++) {
        if (!mspawn_density_has_room(dens, e->category)) break;

        vec3 p = mspawn_pack_member_pos(&pk, i);
        if (!mspawn_density_cell_ok(dens, e->category, p)) continue;

        int id = mob_spawn(mr, e->type, p);
        if (id < 0) break;         // registry full, stop trying

        mspawn_density_account(dens, e->category, p);
        spawned++;
    }

    return spawned;
}
