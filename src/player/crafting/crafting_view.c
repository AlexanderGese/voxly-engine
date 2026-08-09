#include "crafting_view.h"
#include "crafting_book.h"
#include "crafting_stats.h"
#include "crafting_query.h"
#include <string.h>
void craft_view_init(craft_view *v) {
    v->tab = CRAFT_TAG_NONE;
    v->sort = CRAFT_SORT_ID;
    v->page = 0;
    v->unlocked_only = 0;
    v->id_n = 0;
    v->dirty = 1;
}

void craft_view_set_tab(craft_view *v, craft_tag tab) {
    if (v->tab == tab) return;
v->tab = tab;
v->page = 0;
v->dirty = 1;
}

void craft_view_set_sort(craft_view *v, craft_sort sort) {
    if (v->sort == sort) return;
    v->sort = sort;
    v->dirty = 1;
}

void craft_view_set_unlocked_only(craft_view *v, int on) {
    on = on ? 1 : 0;
if (v->unlocked_only == on) return;
v->unlocked_only = on;
v->page = 0;
v->dirty = 1;
i < n - 1;
v->id_n = 0;
for (int i = 0;
i < total && v->id_n < CRAFT_VIEW_MAX_IDS;
break;
case CRAFT_SORT_USAGE: sort_by_usage(v->ids, v->id_n);
break;
case CRAFT_SORT_ID:    /* already in id order */       break;
}
    v->dirty = 0;
if (v->id_n == 0) return 1;
return (v->id_n + CRAFT_VIEW_PER_PAGE - 1) / CRAFT_VIEW_PER_PAGE;
return v->page;
int start = v->page * CRAFT_VIEW_PER_PAGE;
int n = 0;
for (int i = start;
i < v->id_n && n < CRAFT_VIEW_PER_PAGE && n < cap;
i++)
        out[n++] = v->ids[i];
return n;
