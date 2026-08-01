#include "mspawn_pack.h"
#include <math.h>

int mspawn_pack_build(world *w, const mspawn_entry *e,
                      const mspawn_site *center, mspawn_rng *r,
                      mspawn_pack *out) {
    out->center = *center;
    out->count  = 0;

    int want = mspawn_rng_range(r, e->pack_min, e->pack_max);
    if (want < 1) want = 1;
    if (want > MSPAWN_PACK_MAX) want = MSPAWN_PACK_MAX;

    int ccx = (int)floorf(center->pos.x);
    int ccz = (int)floorf(center->pos.z);
    int cfy = center->wy;          // center floor, hint for the members

    // member 0 is the center itself, sitting at the origin offset.
    out->offset[out->count++] = VEC3_ZERO;

    // remaining members: throw a site near the center and keep the relative
    // offset if it validates. a few tries each so a sparse spot still fills
    // when it can without spinning forever.
    for (int m = 1; m < want; m++) {
        int placed = 0;
        for (int attempt = 0; attempt < 4 && !placed; attempt++) {
            mspawn_site s;
            if (!mspawn_try_site(w, e, ccx, ccz, MSPAWN_PACK_RADIUS,
                                 cfy + 2, r, &s))
                continue;

            // skip near-duplicate landings so two mates dont share a tile.
            vec3 off = vec3_sub(s.pos, center->pos);
            int clash = 0;
            for (int k = 0; k < out->count; k++) {
                vec3 d = vec3_sub(off, out->offset[k]);
                if (d.x * d.x + d.z * d.z < 1.0f) { clash = 1; break; }
            }
            if (clash) continue;

            out->offset[out->count++] = off;
            placed = 1;
        }
        // if a member just couldnt find ground, the pack ends up smaller.
        // thats fine, packs near terrain edges should thin out.
    }

    return out->count;
}

vec3 mspawn_pack_member_pos(const mspawn_pack *pk, int i) {
    if (i < 0 || i >= pk->count) return pk->center.pos;
    return vec3_add(pk->center.pos, pk->offset[i]);
}
