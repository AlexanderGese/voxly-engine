#ifndef PLAYER_INVENTORY_FILTER_H
#define PLAYER_INVENTORY_FILTER_H

#include "inv_types.h"
#include "inv_grid.h"

// slot filters: rules for what an item is allowed to live in a given slot.
// the plain bag accepts anything, but armor slots only take armor, the furnace
// fuel slot only takes burnables, etc. ops code asks the filter before letting
// a drop land so the ui can't shove a pickaxe into a helmet slot.
//
// a filter is just a predicate id plus an optional category, kept tiny so a
// grid can carry one per slot without ballooning. NONE = accept everything.

typedef enum {
    INV_FILTER_ANY = 0,    // no restriction
    INV_FILTER_CATEGORY,   // item category must equal `cat`
    INV_FILTER_PLACEABLE,  // item must map to a real place block
    INV_FILTER_FUEL,       // item burns (drives the furnace fuel slot)
    INV_FILTER_TOOL,       // any tool, regardless of tier
} inv_filter_kind;

typedef struct {
    inv_filter_kind kind;
    inv_category    cat;   // only read when kind == INV_FILTER_CATEGORY
} inv_filter;

// the wide-open filter, the default for normal bag slots.
static inline inv_filter inv_filter_any(void) {
    inv_filter f = { INV_FILTER_ANY, INV_CAT_COUNT };
    return f;
}

inv_filter inv_filter_category(inv_category cat);
inv_filter inv_filter_placeable(void);
inv_filter inv_filter_fuel(void);
inv_filter inv_filter_tool(void);

// does this item satisfy the filter? empty/NONE items always pass (so a slot
// can be cleared). this is the single gate the ops layer calls.
int inv_filter_accepts(const inv_filter *f, inv_item_id id);

// convenience: would dropping `src` onto slot `dst_idx` of `g` (gated by the
// matching filter in `filters`, one per slot) be allowed? filters may be NULL
// to mean "all slots accept anything".
int inv_filter_slot_accepts(const inv_filter *filters, const inv_grid *g,
                            int dst_idx, inv_item_id id);

// burn time in ticks for a fuel item, 0 if it doesn't burn. exposed because
// the furnace wants the number, not just the yes/no from the filter.
int inv_filter_fuel_ticks(inv_item_id id);

#endif
