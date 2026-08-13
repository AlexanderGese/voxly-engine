#ifndef PLAYER_INVENTORY_TRANSFER_H
#define PLAYER_INVENTORY_TRANSFER_H
#include "inv_types.h"
#include "inv_grid.h"
#include "inv_filter.h"
// bulk container-to-container moves. inv_ops handles a single slot; this is the
// layer above it that shovels whole items between two grids — what the "loot
// all" / "deposit all" / shift-click-into-chest buttons call. it respects an
// optional per-slot filter array on the destination so you can't dump junk into
// a fuel-only slot.
//
// everything returns the number of items actually moved so the ui can show "+12"
int inv_transfer_item(inv_grid *src, inv_grid *dst,
                      inv_item_id id, int amount,
                      const inv_filter *dst_filters);
int inv_transfer_all(inv_grid *src, inv_grid *dst,
                     const inv_filter *dst_filters);
int inv_transfer_category(inv_grid *src, inv_grid *dst, inv_category cat,
                          const inv_filter *dst_filters);
int inv_transfer_restock(inv_grid *src, inv_grid *dst,
                         const inv_filter *dst_filters);
int inv_transfer_drain_slot(inv_grid *src, int src_idx, inv_grid *dst,
                            const inv_filter *dst_filters);
#endif
