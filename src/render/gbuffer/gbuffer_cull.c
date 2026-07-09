#include "gbuffer_cull.h"
#include "../../util/darray.h"

#include <math.h>

// scratch entry used while sorting. we sort indices+keys, not the lights
// themselves, to keep the swap cheap.
typedef struct {
    float key;   // influence, higher = keep
    int   idx;   // index into the input darray
} cull_entry;

static float influence(const gbuffer_light *li, vec3 eye) {
    if (li->kind == GBUF_LIGHT_SUN) return 1e30f; // sun always wins
    vec3 d = vec3_sub(li->pos, eye);
    float dist2 = vec3_dot(d, d);
    return li->intensity / (1.0f + dist2);
}

// plain insertion sort, descending by key. the list is small (<= a few
// hundred) and mostly pre-sorted frame to frame, so this is fine and avoids
// dragging in qsort + a comparator with global state.
static void sort_desc(cull_entry *e, int n) {
    for (int i = 1; i < n; i++) {
        cull_entry t = e[i];
        int j = i - 1;
        while (j >= 0 && e[j].key < t.key) {
            e[j + 1] = e[j];
            j--;
        }
        e[j + 1] = t;
    }
}

void gbuffer_cull(const gbuffer_light_list *in, const frustum *fr,
                  vec3 eye, gbuffer_cull_result *out) {
    out->count   = 0;
    out->dropped = 0;

    // the sun is special: it's not in the darray and never gets culled.
    if (in->sun_set && out->count < GBUFFER_MAX_LIGHTS)
        out->lights[out->count++] = in->sun;

    int n = (int)darr_len(in->items);
    if (n <= 0) return;

    // build candidate list, frustum-rejecting point lights up front
    static cull_entry scratch[4096];
    int cand = 0;
    for (int i = 0; i < n && cand < (int)(sizeof scratch / sizeof *scratch); i++) {
        const gbuffer_light *li = &in->items[i];
        if (fr) {
            aabb b = gbuffer_light_bounds(li);
            if (!frustum_contains_aabb(fr, b)) continue;
        }
        scratch[cand].key = influence(li, eye);
        scratch[cand].idx = i;
        cand++;
    }

    sort_desc(scratch, cand);

    for (int i = 0; i < cand; i++) {
        if (out->count >= GBUFFER_MAX_LIGHTS) {
            out->dropped = cand - i;
            break;
        }
        out->lights[out->count++] = in->items[scratch[i].idx];
    }
}
