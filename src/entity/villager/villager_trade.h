#ifndef ENTITY_VILLAGER_TRADE_H
#define ENTITY_VILLAGER_TRADE_H
#include "villager_types.h"
#include "villager_gossip.h"
#include "../../world/block.h"
#include <stdint.h>
// trade offers. a villager exposes up to MAX_OFFERS slots; each is a "give
// these in items, get this item out" recipe with a stock counter that
// depletes as the player buys and refills when the villager works.
//
// the engine only has block items, so goods are block_ids. we pick one block
// to act as the village currency (the "emerald") below — see villager_trade.c.
//
// prices are nudged by reputation (via gossip) and by demand: heavily-used
// offers drift more expensive, idle ones drift back down. classic.
#define VILLAGER_MAX_OFFERS 6
typedef struct {
    block_id want;          // item the player pays in
    int      want_count;    // base price in `want`
    block_id give;          // item the villager hands over
    int      give_count;    // how many per trade
    int      uses;          // times traded since last restock
    int      max_uses;      // restock cap before it locks
    int      xp;            // villager xp gained per trade (drives leveling)
    int8_t   demand;        // -N..+N price pressure, drifts toward 0
    uint8_t  unlocked;      // visible to the player yet?
    uint8_t  locked;        // depleted: needs restock to re-open
} villager_offer;
typedef struct {
    villager_offer offers[VILLAGER_MAX_OFFERS];
    int            count;
    int            level;       // 1..5, gates which offers unlock
    int            xp;          // accumulated trade xp
} villager_trades;
// build the offer table for a profession at level 1. clears everything.
void villager_trades_init(villager_trades *t, villager_profession prof);
// current price for an offer slot, after reputation + demand. >=1 always.
int  villager_trade_price(const villager_trades *t, int slot,
                          const villager_gossip *g);
int  villager_trade_can(const villager_trades *t, int slot, int player_has);
int  villager_trade_do(villager_trades *t, int slot, villager_gossip *g,
                       block_id *out_item, int *out_count, int *paid);
int  villager_trade_restock(villager_trades *t, int amount);
#endif
