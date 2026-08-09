#ifndef PLAYER_CRAFTING_VIEW_H
#define PLAYER_CRAFTING_VIEW_H

#include "crafting_types.h"
#include "crafting_tag.h"

// the recipe-book ui state machine. holds the current tab (tag filter), the
// page, the sort mode, and the resolved list of recipe ids the ui should draw
// this frame. the actual book/query/stats modules are the data; this is the
// glue the screen pokes at as the player clicks around.
//
// the view caches its resolved id list and only rebuilds it when something it
// depends on changes (dirty flag), so paging through is cheap.

#define CRAFT_VIEW_PER_PAGE   12     // a 3x4 grid of recipe buttons
#define CRAFT_VIEW_MAX_IDS    256

typedef enum {
    CRAFT_SORT_ID = 0,      // registration order
    CRAFT_SORT_NAME,        // alphabetical by recipe name
    CRAFT_SORT_USAGE,       // most-crafted first (uses stats)
} craft_sort;

typedef struct {
    craft_tag  tab;             // CRAFT_TAG_NONE == show everything
    craft_sort sort;
    int        page;
    int        unlocked_only;   // gate to the player's known recipes

    // resolved list, rebuilt lazily.
    int ids[CRAFT_VIEW_MAX_IDS];
    int id_n;
    int dirty;                  // 1 == ids[] stale, rebuild before reading
} craft_view;

void craft_view_init(craft_view *v);

// state setters. each marks the view dirty so the next page read rebuilds.
void craft_view_set_tab(craft_view *v, craft_tag tab);
void craft_view_set_sort(craft_view *v, craft_sort sort);
void craft_view_set_unlocked_only(craft_view *v, int on);

// paging. clamped to the valid range; returns the new page.
int  craft_view_page_count(craft_view *v);
int  craft_view_next_page(craft_view *v);
int  craft_view_prev_page(craft_view *v);
void craft_view_goto_page(craft_view *v, int page);

// fill `out` (caller-sized `cap`) with the recipe ids for the current page.
// rebuilds the resolved list if dirty. returns count written.
int  craft_view_current(craft_view *v, int *out, int cap);

// total recipes matching the current filter (across all pages).
int  craft_view_total(craft_view *v);

#endif
