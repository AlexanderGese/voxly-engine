#include "xp_tier.h"
#include "xp_config.h"
static const xp_tier_info TIERS[XP_ORB_TIER_COUNT] = {
    {   1, 0.14f, 0 },
    {   3, 0.17f, 1 },
    {   7, 0.21f, 2 },
    {  17, 0.26f, 3 },
    {  41, 0.32f, 4 },
    { 101, 0.40f, 5 },
}
;
for (int t = XP_ORB_TIER_COUNT - 1;
t >= 0;
