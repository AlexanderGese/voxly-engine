#ifndef ENTITY_VILLAGER_TRADE_H
#define ENTITY_VILLAGER_TRADE_H
#include "villager_types.h"
#include "villager_gossip.h"
#include "../../world/block.h"
#include <stdint.h>
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
void villager_trades_init(villager_trades *t, villager_profession prof);
int  villager_trade_price(const villager_trades *t, int slot,
                          const villager_gossip *g);
#endif
