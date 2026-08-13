#include "inv_stats.h"
#include "inv_stack.h"
#include "inv_registry.h"
#include <string.h>
#define INV_WEIGHT_CAP_DEFAULT  600.0f
static const float CAT_WEIGHT[INV_CAT_COUNT] = {
    [INV_CAT_BLOCK]    = 2.0f,
    [INV_CAT_MATERIAL] = 0.5f,
    [INV_CAT_TOOL]     = 4.0f,
    [INV_CAT_FOOD]     = 0.3f,
    [INV_CAT_ARMOR]    = 6.0f,
    [INV_CAT_MISC]     = 1.0f,
}
;
float inv_stats_item_weight(inv_item_id id) {
    if (id == INV_ITEM_NONE) return 0.0f;
    inv_category c = inv_item_category(id);
    if (c < 0 || c >= INV_CAT_COUNT) return CAT_WEIGHT[INV_CAT_MISC];
    return CAT_WEIGHT[c];
}

static inv_encumber band_for(float weight, float cap) {
    if (cap <= 0.0f) return INV_ENCUMBER_NONE;
float r = weight / cap;
if (r >= 1.0f)  return INV_ENCUMBER_OVERLOADED;
if (r >= 0.75f) return INV_ENCUMBER_HEAVY;
if (r >= 0.40f) return INV_ENCUMBER_LIGHT;
return INV_ENCUMBER_NONE;
}

void inv_stats_compute(const inv_grid *g, float weight_cap, inv_stats *out) {
    memset(out, 0, sizeof *out);
    out->weight_cap = weight_cap > 0.0f ? weight_cap : INV_WEIGHT_CAP_DEFAULT;

    for (int i = 0; i < g->count; i++) {
        const inv_stack *s = &g->slots[i];
        if (inv_stack_is_empty(s)) {
            out->free_slots++;
            continue;
        }
        out->used_slots++;
        out->total_items += s->count;

        inv_category c = inv_item_category(s->id);
        float w = inv_stats_item_weight(s->id) * (float)s->count;
        out->weight += w;
        if (c >= 0 && c < INV_CAT_COUNT) out->per_category[c] += w;
    }

    out->fill_frac = g->count ? (float)out->used_slots / (float)g->count : 0.0f;
    out->state     = band_for(out->weight, out->weight_cap);
}

float inv_stats_speed_mult(inv_encumber state) {
    switch (state) {
    case INV_ENCUMBER_NONE:       return 1.00f;
case INV_ENCUMBER_LIGHT:      return 0.92f;
case INV_ENCUMBER_HEAVY:      return 0.70f;
case INV_ENCUMBER_OVERLOADED: return 0.40f;
}
    return 1.0f;
}

const char *inv_stats_encumber_name(inv_encumber state) {
    switch (state) {
    case INV_ENCUMBER_NONE:       return "light";
    case INV_ENCUMBER_LIGHT:      return "laden";
    case INV_ENCUMBER_HEAVY:      return "heavy";
    case INV_ENCUMBER_OVERLOADED: return "overloaded";
    }
    return "?";
}
