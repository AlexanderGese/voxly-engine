#include "villager_trade.h"
#include "villager_def.h"

#include <stddef.h>
#include <string.h>

// no emerald block exists in this engine, so glass plays the role of the
// shiny village currency. it's transparent and slightly precious-looking;
// good enough for a barter economy.
#define VILLAGER_CURRENCY BLOCK_GLASS

// xp needed to reach each level (index = level-1). level 5 is the cap.
static const int level_xp[5] = { 0, 10, 30, 70, 150 };

// demand clamp. beyond this the price won't keep climbing.
#define DEMAND_LIMIT 8

// add an offer if there's room. returns the slot or -1.
static int push_offer(villager_trades *t, block_id want, int wc,
                      block_id give, int gc, int max_uses, int xp) {
    if (t->count >= VILLAGER_MAX_OFFERS) return -1;
    villager_offer *o = &t->offers[t->count];
    memset(o, 0, sizeof *o);
    o->want = want; o->want_count = wc;
    o->give = give; o->give_count = gc;
    o->max_uses = max_uses;
    o->uses = 0;
    o->xp = xp;
    o->demand = 0;
    o->locked = 0;
    o->unlocked = 0;   // unlock step happens after, based on level
    return t->count++;
}

// build the per-profession recipe book. each profession sells its produce
// for currency and buys back a raw good. these tables are intentionally
// short — flavor over realism.
static void build_offers(villager_trades *t, villager_profession prof) {
    switch (prof) {
    case VILLAGER_PROF_FARMER:
        push_offer(t, VILLAGER_CURRENCY, 1, BLOCK_GRASS, 18, 12, 2);
        push_offer(t, BLOCK_DIRT, 20, VILLAGER_CURRENCY, 1, 16, 2);
        push_offer(t, VILLAGER_CURRENCY, 3, BLOCK_LEAVES, 4, 8, 5);
        break;
    case VILLAGER_PROF_LIBRARIAN:
        push_offer(t, BLOCK_WOOD, 12, VILLAGER_CURRENCY, 1, 16, 2);
        push_offer(t, VILLAGER_CURRENCY, 2, BLOCK_PLANKS, 6, 12, 3);
        push_offer(t, VILLAGER_CURRENCY, 5, BLOCK_GLASS, 1, 6, 8);
        break;
    case VILLAGER_PROF_BLACKSMITH:
        push_offer(t, BLOCK_STONE, 10, VILLAGER_CURRENCY, 1, 16, 2);
        push_offer(t, VILLAGER_CURRENCY, 4, BLOCK_BRICK, 4, 10, 4);
        push_offer(t, VILLAGER_CURRENCY, 6, BLOCK_TORCH, 8, 8, 6);
        break;
    case VILLAGER_PROF_BUTCHER:
        push_offer(t, BLOCK_GRASS, 14, VILLAGER_CURRENCY, 1, 16, 2);
        push_offer(t, VILLAGER_CURRENCY, 2, BLOCK_COBBLE, 8, 12, 3);
        break;
    case VILLAGER_PROF_MASON:
        push_offer(t, BLOCK_COBBLE, 12, VILLAGER_CURRENCY, 1, 16, 2);
        push_offer(t, VILLAGER_CURRENCY, 3, BLOCK_STONE, 6, 12, 4);
        push_offer(t, VILLAGER_CURRENCY, 4, BLOCK_BRICK, 6, 8, 5);
        break;
    case VILLAGER_PROF_CLERIC:
        push_offer(t, VILLAGER_CURRENCY, 3, BLOCK_TORCH, 4, 12, 4);
        push_offer(t, BLOCK_SAND, 10, VILLAGER_CURRENCY, 1, 12, 3);
        break;
    default:
        // unemployed / nitwit: nothing to sell. empty book.
        break;
    }
}

void villager_trades_init(villager_trades *t, villager_profession prof) {
    memset(t, 0, sizeof *t);
    t->level = 1;
    t->xp = 0;
    build_offers(t, prof);

    // unlock the first few offers per the profession def; the rest gate
    // behind trade levels and open up in apply_level().
    int unlock = villager_def_get(prof)->base_offers;
    if (unlock > t->count) unlock = t->count;
    for (int i = 0; i < t->count; i++)
        t->offers[i].unlocked = (i < unlock) ? 1 : 0;
}

int villager_trade_price(const villager_trades *t, int slot,
                         const villager_gossip *g) {
    if (slot < 0 || slot >= t->count) return 1;
    const villager_offer *o = &t->offers[slot];

    float base = (float)o->want_count;
    // reputation discount/markup
    base *= villager_gossip_price_mult(g);
    // demand pressure: each demand point shifts price ~7%
    base *= 1.0f + 0.07f * (float)o->demand;

    int price = (int)(base + 0.5f);
    if (price < 1) price = 1;
    return price;
}

int villager_trade_can(const villager_trades *t, int slot, int player_has) {
    if (slot < 0 || slot >= t->count) return 0;
    const villager_offer *o = &t->offers[slot];
    if (!o->unlocked || o->locked) return 0;
    if (o->uses >= o->max_uses) return 0;
    return player_has >= o->want_count;   // caller prices via _price first
}

// re-evaluate level from xp and unlock any offers that just became available.
static void apply_level(villager_trades *t) {
    int lvl = 1;
    for (int i = 4; i >= 0; i--) {
        if (t->xp >= level_xp[i]) { lvl = i + 1; break; }
    }
    if (lvl > t->level) t->level = lvl;

    // unlock count grows with level: 2 at L1, +1 per level after.
    int unlock = 1 + t->level;
    if (unlock > t->count) unlock = t->count;
    for (int i = 0; i < unlock; i++) t->offers[i].unlocked = 1;
}

int villager_trade_do(villager_trades *t, int slot, villager_gossip *g,
                      block_id *out_item, int *out_count, int *paid) {
    if (slot < 0 || slot >= t->count) return 0;
    villager_offer *o = &t->offers[slot];
    if (!o->unlocked || o->locked || o->uses >= o->max_uses) return 0;

    *paid      = villager_trade_price(t, slot, g);
    *out_item  = o->give;
    *out_count = o->give_count;

    o->uses++;
    if (o->demand < DEMAND_LIMIT) o->demand++;
    if (o->uses >= o->max_uses) o->locked = 1;

    // trading earns the player goodwill and the villager xp.
    villager_gossip_add(g, VILLAGER_GOSSIP_TRADING, 1);
    t->xp += o->xp;
    apply_level(t);
    return 1;
}

int villager_trade_restock(villager_trades *t, int amount) {
    int reopened = 0;
    for (int i = 0; i < t->count; i++) {
        villager_offer *o = &t->offers[i];
        if (o->uses == 0 && !o->locked) {
            // idle offer: let demand relax back toward neutral.
            if (o->demand > 0) o->demand--;
            continue;
        }
        int refill = amount;
        if (refill > o->uses) refill = o->uses;
        o->uses -= refill;
        if (o->locked && o->uses < o->max_uses) {
            o->locked = 0;
            reopened++;
        }
        if (o->demand > 0) o->demand--;
    }
    return reopened;
}
