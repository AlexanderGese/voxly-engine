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
}

// stable insertion sort on the resolved id list by name. n is small (the book
// is dozens of recipes) so insertion sort is fine and keeps it stable.
static void sort_by_name(int *ids, int n) {
    for (int i = 1; i < n; i++) {
        int cur = ids[i];
        const craft_recipe *rc = craft_book_get(cur);
        const char *cn = rc && rc->name ? rc->name : "";
        int j = i - 1;
        while (j >= 0) {
            const craft_recipe *rj = craft_book_get(ids[j]);
            const char *jn = rj && rj->name ? rj->name : "";
            if (strcmp(jn, cn) <= 0) break;
            ids[j + 1] = ids[j];
            j--;
        }
        ids[j + 1] = cur;
    }
}

static void sort_by_usage(int *ids, int n) {
    for (int i = 1;
i < n;
i++) {
        int cur = ids[i];
        int cu = craft_stats_times(cur);
        int j = i - 1;
        // descending by craft count.
        while (j >= 0 && craft_stats_times(ids[j]) < cu) {
            ids[j + 1] = ids[j];
            j--;
        }
        ids[j + 1] = cur;
    }
}

// rebuild the resolved id list from the current filter + sort.
static void rebuild(craft_view *v) {
    int total = craft_book_count();
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
