#include "xp_drop.h"

#include "xp_config.h"
#include "xp_orb.h"
#include "xp_tier.h"
#include "../../entity/entity.h"

int xp_drop_value_for_mob(int entity_type) {
    // vanilla-ish numbers. hostiles pay more than passives, naturally.
    switch (entity_type) {
    case ET_ZOMBIE:   return 5;
    case ET_SKELETON: return 5;
    case ET_SPIDER:   return 5;
    case ET_COW:      return 2;
    case ET_PIG:      return 2;
    default:          return 1; // a participation trophy
    }
}

// per-source value multiplier. ore/smelt feel chunkier; breeding is small.
static int source_scale(int amount, xp_source src) {
    switch (src) {
    case XP_SRC_ORE:    return amount + amount / 2; // 1.5x, ore is satisfying
    case XP_SRC_SMELT:  return amount;
    case XP_SRC_BREED:  return amount < 7 ? amount : 7; // capped, no farms
    case XP_SRC_BOTTLE: return amount;
    case XP_SRC_DEBUG:  return amount;
    case XP_SRC_MOB:
    default:            return amount;
    }
}

int xp_drop_spawn(xp_orb_pool *pool, vec3 pos, int amount, xp_source src) {
    if (!pool || amount <= 0) return 0;

    int remaining = source_scale(amount, src);
    int spawned = 0;

    // greedily bite off the largest tier that fits until we're out. this
    // keeps orb counts tiny: a 101-value drop is one fat orb, not 101 motes.
    int guard = 0;
    while (remaining > 0) {
        int tier = xp_tier_pick(remaining);
        if (tier < 0) break;
        int v = xp_tier_get(tier)->value;

        if (xp_orb_spawn(pool, pos, tier) < 0)
            break; // pool full, abandon the rest

        remaining -= v;
        spawned++;

        // hard stop so a pathological amount can't spin forever; also caps
        // the orb burst from a single event to something reasonable.
        if (++guard > 64) break;
    }
    return spawned;
}
