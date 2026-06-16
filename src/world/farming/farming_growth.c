#include "farming_growth.h"
#include "farming_def.h"
#include "farming_block.h"
#include "../../config.h"
// light below this and the crop simply refuses to grow. matches the engine's
// "needs a bit of sky" feel without being a hard cliff at full dark.
#define GROW_MIN_LIGHT   7
float farming_growth_neighbor_bonus(world *w, int wx, int wy, int wz,
                                    farming_crop_kind kind) {
    block_id self = farming_block_for_crop(kind);
    if (self == BLOCK_AIR) return 0.0f;

    // minecraft-ish: each matching cardinal neighbor adds 1, each diagonal pair
    // alignment adds a bit, then we normalise into 0..1. rows beat clumps.
    float score = 0.0f;
    static const int card[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    int card_match = 0;
    for (int i = 0; i < 4; i++) {
        block_id b = world_get_block(w, wx + card[i][0], wy, wz + card[i][1]);
        if (b == self) { score += 1.0f; card_match++; }
    }

    // diagonals only count when they reinforce a straight row through us, i.e.
    // both blocks on an axis match. cheap way to reward tidy rows.
    int ew = (world_get_block(w, wx + 1, wy, wz) == self) &&
             (world_get_block(w, wx - 1, wy, wz) == self);
    int ns = (world_get_block(w, wx, wy, wz + 1) == self) &&
             (world_get_block(w, wx, wy, wz - 1) == self);
    if (ew) score += 0.5f;
    if (ns) score += 0.5f;
    (void)card_match;

    // max raw score is 4 + 1 = 5. clamp/normalise to 0..1.
    float f = score / 5.0f;
    return f > 1.0f ? 1.0f : f;
}

int farming_growth_tick(farming_crop *crop, const farming_def *def,
                        const farming_env *env, uint32_t seed) {
    if (!(crop->flags & FARMING_CROP_F_ALIVE)) return 0;
if (crop->flags & FARMING_CROP_F_MATURE)  return 0;
if (!env->supported) return 0;
// wilting crops dont grow; handled elsewhere
if (env->light < GROW_MIN_LIGHT) return 0;
// base gain scaled by environment. neighbor bonus is additive on top of a
// baseline 1.0 so a lone crop still grows at full rate, rows just go faster.
float gain = def->base_rate;
gain *= env->hydration;
gain *= (1.0f + 0.5f * env->neighbor);
// a little stochastic jitter so a whole field doesnt tick in lockstep. roll
// a deterministic 0.75..1.25 multiplier from the salted hash.
farming_rng rr;
farming_rng_seed(&rr, farming_hash3(crop->wx, crop->wy, crop->wz, seed));
gain *= 0.75f + 0.5f * farming_rng_f01(&rr);
crop->growth_accum += gain;
int advanced = 0;
int top = (int)def->max_stage;
while (crop->growth_accum >= def->points_per_stage && crop->stage < top) {
        crop->growth_accum -= def->points_per_stage;
        crop->stage++;
        advanced = 1;
    }

    if (crop->stage >= top) {
        crop->stage = (uint8_t)top;
crop->flags |= FARMING_CROP_F_MATURE;
crop->growth_accum = 0.0f;
// mature crops bank no overflow
}
    return advanced;
}

int farming_growth_is_mature(const farming_crop *crop, const farming_def *def) {
    return crop->stage >= def->max_stage;
}
