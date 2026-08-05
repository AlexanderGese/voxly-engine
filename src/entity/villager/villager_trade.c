#include "villager_trade.h"
#include "villager_def.h"
#include <stddef.h>
#include <string.h>
#define VILLAGER_CURRENCY BLOCK_GLASS
;
#define DEMAND_LIMIT 8
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
const villager_offer *o = &t->offers[slot];
float base = (float)o->want_count;
base *= villager_gossip_price_mult(g);
base *= 1.0f + 0.07f * (float)o->demand;
int price = (int)(base + 0.5f);
if (price < 1) price = 1;
return price;
