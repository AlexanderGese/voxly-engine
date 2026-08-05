#ifndef ENTITY_VILLAGER_GOSSIP_H
#define ENTITY_VILLAGER_GOSSIP_H

#include "villager_types.h"
#include <stdint.h>

// per-villager reputation ledger toward a single player. we only model one
// player in this engine so the "target" is implicit. each gossip kind has
// its own bucket; the net reputation is a weighted sum, clamped, and feeds
// the trade pricing as a discount/markup.

typedef struct {
    int16_t bucket[VILLAGER_GOSSIP_COUNT];
    float   decay_accum;   // seconds-into-day accumulator for daily decay
} villager_gossip;

void villager_gossip_init(villager_gossip *g);

// push an event of `amount` magnitude (positive ints). internally clamped to
// per-kind caps. negative kinds still take positive amounts; the sign is in
// the kind, not the amount.
void villager_gossip_add(villager_gossip *g, villager_gossip_kind kind, int amount);

// net reputation in roughly [-VILLAGER_GOSSIP_MAX, +VILLAGER_GOSSIP_MAX].
// positive = player is liked, negative = player is on thin ice.
int  villager_gossip_reputation(const villager_gossip *g);

// price multiplier from reputation. 1.0 = neutral. liked players pay less
// (down to ~0.7), disliked pay more (up to ~1.3). monotonic, clamped.
float villager_gossip_price_mult(const villager_gossip *g);

// shed a little reputation over time so old grudges (and old goodwill) fade.
// call each tick with dt seconds.
void villager_gossip_tick(villager_gossip *g, float dt);

// share gossip between two villagers who meet (the "GATHER" activity uses
// this). both ledgers drift toward their average — rumors spread.
void villager_gossip_share(villager_gossip *a, villager_gossip *b);

#endif
