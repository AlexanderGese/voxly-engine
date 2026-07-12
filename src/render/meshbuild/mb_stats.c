#include "mb_stats.h"

static mb_stats g_stats;

void mb_stats_reset(void) {
    g_stats.builds = 0;
    g_stats.quads = 0;
    g_stats.faces_merged = 0;
    g_stats.verts = 0;
    g_stats.last_ms = 0.0;
    g_stats.total_ms = 0.0;
}

mb_stats mb_stats_get(void) {
    return g_stats;
}

void mb_stats_record(int quads, int merged, int verts, double ms) {
    g_stats.builds       += 1;
    g_stats.quads        += (uint64_t)quads;
    g_stats.faces_merged += (uint64_t)merged;
    g_stats.verts        += (uint64_t)verts;
    g_stats.last_ms       = ms;
    g_stats.total_ms     += ms;
}

double mb_stats_merge_ratio(void) {
    if (g_stats.quads == 0) return 0.0;
    return (double)g_stats.faces_merged / (double)g_stats.quads;
}
