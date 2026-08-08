#include "crafting_stats.h"
#include <string.h>

static int s_times[CRAFT_STATS_CAP];
static int s_yield[CRAFT_STATS_CAP];
static int s_total;

// recent ring. s_recent[0] is the oldest slot index-wise but we track head so
// "newest first" reads are easy. -1 == empty slot.
static int s_recent[CRAFT_RECENT_RING];
static int s_recent_n;     // how many slots are populated (<= ring size)

void craft_stats_init(void) {
    craft_stats_reset();
}

void craft_stats_reset(void) {
    memset(s_times, 0, sizeof s_times);
    memset(s_yield, 0, sizeof s_yield);
    s_total = 0;
    for (int i = 0; i < CRAFT_RECENT_RING; i++) s_recent[i] = -1;
    s_recent_n = 0;
}

// push id to the front of the recent list, de-duping. simple shift since the
// ring is tiny (8). if id is already present we pull it out first so it ends
// up at the front without occupying two slots.
static void recent_push(int id) {
    int src = 0;
    // build a fresh front-loaded order skipping any existing copy.
    int tmp[CRAFT_RECENT_RING];
    int n = 0;
    tmp[n++] = id;
    for (int i = 0; i < s_recent_n && n < CRAFT_RECENT_RING; i++) {
        if (s_recent[i] == id) continue;
        tmp[n++] = s_recent[i];
    }
    (void)src;
    for (int i = 0; i < n; i++) s_recent[i] = tmp[i];
    for (int i = n; i < CRAFT_RECENT_RING; i++) s_recent[i] = -1;
    s_recent_n = n;
}

void craft_stats_record(int recipe_id, int made) {
    if (recipe_id < 0 || recipe_id >= CRAFT_STATS_CAP) return;
    if (made < 0) made = 0;
    s_times[recipe_id]++;
    s_yield[recipe_id] += made;
    s_total++;
    recent_push(recipe_id);
}

int craft_stats_times(int recipe_id) {
    if (recipe_id < 0 || recipe_id >= CRAFT_STATS_CAP) return 0;
    return s_times[recipe_id];
}

int craft_stats_yield(int recipe_id) {
    if (recipe_id < 0 || recipe_id >= CRAFT_STATS_CAP) return 0;
    return s_yield[recipe_id];
}

int craft_stats_favorite(void) {
    int best = -1, best_n = 0;
    for (int i = 0; i < CRAFT_STATS_CAP; i++) {
        if (s_times[i] > best_n) { best_n = s_times[i]; best = i; }
    }
    return best;
}

// selection-sort the populated ids by count desc into `out`. n is small so the
// O(n^2) doesnt matter and we avoid allocating a scratch index array.
int craft_stats_top(int *out, int cap) {
    char used[CRAFT_STATS_CAP];
    memset(used, 0, sizeof used);
    int n = 0;
    while (n < cap) {
        int best = -1, best_n = 0;
        for (int i = 0; i < CRAFT_STATS_CAP; i++) {
            if (used[i] || s_times[i] == 0) continue;
            if (s_times[i] > best_n) { best_n = s_times[i]; best = i; }
        }
        if (best < 0) break;
        used[best] = 1;
        out[n++] = best;
    }
    return n;
}

int craft_stats_recent(int *out, int cap) {
    int n = 0;
    for (int i = 0; i < s_recent_n && n < cap; i++) {
        if (s_recent[i] < 0) break;
        out[n++] = s_recent[i];
    }
    return n;
}

int craft_stats_total_crafts(void) { return s_total; }
