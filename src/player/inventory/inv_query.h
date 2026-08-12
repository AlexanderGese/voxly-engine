#ifndef PLAYER_INVENTORY_QUERY_H
#define PLAYER_INVENTORY_QUERY_H
#include "inv_types.h"
#include "inv_grid.h"
int inv_query_count(const inv_grid *g, inv_item_id id);
int inv_query_slots_with(const inv_grid *g, inv_item_id id);
int inv_query_count_category(const inv_grid *g, inv_category cat);
int inv_query_room_for(const inv_grid *g, inv_item_id id);
int inv_query_fits(const inv_grid *g, inv_item_id id, int amount);
int inv_query_is_empty(const inv_grid *g);
#endif
