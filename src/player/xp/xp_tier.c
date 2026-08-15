#include "xp_tier.h"

#include "xp_config.h"

// hand-balanced. values roughly double each tier so the greedy split stays
// short. radii grow gently so a fat orb reads as "worth grabbing".
static const xp_tier_info TIERS[XP_ORB_TIER_COUNT] = {
    {   1, 0.14f, 0 },
    {   3, 0.17f, 1 },
    {   7, 0.21f, 2 },
    {  17, 0.26f, 3 },
    {  41, 0.32f, 4 },
    { 101, 0.40f, 5 },
};

const xp_tier_info *xp_tier_get(int tier) {
    if (tier < 0) tier = 0;
    if (tier >= XP_ORB_TIER_COUNT) tier = XP_ORB_TIER_COUNT - 1;
    return &TIERS[tier];
}

int xp_tier_pick(int remaining) {
    if (remaining <= 0) return -1;
    // walk down from the top tier, take the first that fits.
    for (int t = XP_ORB_TIER_COUNT - 1; t >= 0; t--) {
        if (TIERS[t].value <= remaining)
            return t;
    }
    // remaining is positive but smaller than the cheapest tier (== 1), which
    // can't happen, but the compiler doesn't know that.
    return 0;
}

vec3 xp_tier_color(int tier) {
    const xp_tier_info *ti = xp_tier_get(tier);
    // lerp green -> yellow -> orange across the tint range. classic xp green.
    float t = (float)ti->tint / (float)(XP_ORB_TIER_COUNT - 1);
    vec3 lo  = vec3_new(0.45f, 0.95f, 0.30f); // green
    vec3 mid = vec3_new(0.95f, 0.95f, 0.25f); // yellow
    vec3 hi  = vec3_new(1.00f, 0.65f, 0.15f); // orange
    if (t < 0.5f) {
        float k = t * 2.0f;
        return vec3_lerp(lo, mid, k);
    } else {
        float k = (t - 0.5f) * 2.0f;
        return vec3_lerp(mid, hi, k);
    }
}
