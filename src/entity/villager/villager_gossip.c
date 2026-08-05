#include "villager_gossip.h"

#include <stddef.h>

// seconds in one in-game day. the daynight system has its own clock but we
// don't want a hard dependency on it just for decay timing, so we keep a
// local constant. ~20 minutes feels villagery.
#define GOSSIP_DAY_SECONDS 1200.0f

// per-kind ceilings. the negatives are allowed to bite harder than the
// positives reward — being a jerk should be expensive.
static const int kind_cap[VILLAGER_GOSSIP_COUNT] = {
    [VILLAGER_GOSSIP_MINOR_POSITIVE] = 20,
    [VILLAGER_GOSSIP_MAJOR_POSITIVE] = 80,
    [VILLAGER_GOSSIP_TRADING]        = 25,
    [VILLAGER_GOSSIP_MINOR_NEGATIVE] = 40,
    [VILLAGER_GOSSIP_MAJOR_NEGATIVE] = 100,
};

// sign of each bucket when summed into net reputation.
static const int kind_sign[VILLAGER_GOSSIP_COUNT] = {
    [VILLAGER_GOSSIP_MINOR_POSITIVE] = +1,
    [VILLAGER_GOSSIP_MAJOR_POSITIVE] = +1,
    [VILLAGER_GOSSIP_TRADING]        = +1,
    [VILLAGER_GOSSIP_MINOR_NEGATIVE] = -1,
    [VILLAGER_GOSSIP_MAJOR_NEGATIVE] = -1,
};

void villager_gossip_init(villager_gossip *g) {
    for (int i = 0; i < VILLAGER_GOSSIP_COUNT; i++) g->bucket[i] = 0;
    g->decay_accum = 0.0f;
}

void villager_gossip_add(villager_gossip *g, villager_gossip_kind kind, int amount) {
    if (kind < 0 || kind >= VILLAGER_GOSSIP_COUNT) return;
    if (amount < 0) amount = 0;
    int v = g->bucket[kind] + amount;
    if (v > kind_cap[kind]) v = kind_cap[kind];
    g->bucket[kind] = (int16_t)v;
}

int villager_gossip_reputation(const villager_gossip *g) {
    int net = 0;
    for (int i = 0; i < VILLAGER_GOSSIP_COUNT; i++)
        net += kind_sign[i] * (int)g->bucket[i];
    if (net >  VILLAGER_GOSSIP_MAX) net =  VILLAGER_GOSSIP_MAX;
    if (net < -VILLAGER_GOSSIP_MAX) net = -VILLAGER_GOSSIP_MAX;
    return net;
}

float villager_gossip_price_mult(const villager_gossip *g) {
    int   rep = villager_gossip_reputation(g);
    // map [-MAX,+MAX] -> [1.3, 0.7]. positive rep => cheaper.
    float t = (float)rep / (float)VILLAGER_GOSSIP_MAX;   // [-1,1]
    float mult = 1.0f - 0.3f * t;
    if (mult < 0.7f) mult = 0.7f;
    if (mult > 1.3f) mult = 1.3f;
    return mult;
}

void villager_gossip_tick(villager_gossip *g, float dt) {
    g->decay_accum += dt;
    if (g->decay_accum < GOSSIP_DAY_SECONDS) return;
    g->decay_accum -= GOSSIP_DAY_SECONDS;

    // a day passed. shed a flat chunk from every bucket toward zero.
    for (int i = 0; i < VILLAGER_GOSSIP_COUNT; i++) {
        int v = g->bucket[i];
        if (v > VILLAGER_GOSSIP_DECAY_DAY) v -= VILLAGER_GOSSIP_DECAY_DAY;
        else                               v = 0;
        g->bucket[i] = (int16_t)v;
    }
}

void villager_gossip_share(villager_gossip *a, villager_gossip *b) {
    // rumors propagate: each bucket drifts a quarter of the way toward the
    // other's value. not the full average — gossip is lossy.
    for (int i = 0; i < VILLAGER_GOSSIP_COUNT; i++) {
        int av = a->bucket[i], bv = b->bucket[i];
        int da = (bv - av) / 4;
        int db = (av - bv) / 4;
        int na = av + da, nb = bv + db;
        if (na > kind_cap[i]) na = kind_cap[i];
        if (nb > kind_cap[i]) nb = kind_cap[i];
        if (na < 0) na = 0;
        if (nb < 0) nb = 0;
        a->bucket[i] = (int16_t)na;
        b->bucket[i] = (int16_t)nb;
    }
}
