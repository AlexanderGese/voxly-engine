#include "tools_stats.h"
#include "tools_registry.h"
#include <string.h>

void tools_stats_reset(tools_stats *s) {
    memset(s, 0, sizeof *s);
}

void tools_stats_on_break(tools_stats *s, const tool_item *t, block_id block,
                          int dura_cost, int snapped, int xp) {
    s->blocks_broken++;

    mat_class cls = tools_registry_class(block);
    if (cls >= 0 && cls < MAT_CLASS_COUNT) s->by_material[cls]++;

    tool_kind k = t->head.kind;
    if (k >= 0 && k < TOOL_KIND_COUNT) s->by_tool[k]++;

    if (dura_cost > 0) s->durability_spent += dura_cost;
    if (snapped)       s->tools_snapped++;
    if (xp > 0)        s->ore_xp += xp;
}

void tools_stats_tick(tools_stats *s, float dt) {
    if (dt > 0.0f) s->seconds_digging += dt;
}

float tools_stats_wear_rate(const tools_stats *s) {
    if (s->blocks_broken <= 0) return 0.0f;
    return (float)s->durability_spent / (float)s->blocks_broken;
}

mat_class tools_stats_top_material(const tools_stats *s) {
    mat_class best = MAT_NONE;
    long      bestn = 0;
    // start at 1: MAT_NONE is air, never interesting to report as "top".
    for (int i = 1; i < MAT_CLASS_COUNT; i++) {
        if (s->by_material[i] > bestn) {
            bestn = s->by_material[i];
            best  = (mat_class)i;
        }
    }
    return best;
}
