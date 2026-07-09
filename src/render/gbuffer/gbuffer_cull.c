#include "gbuffer_cull.h"
#include "../../util/darray.h"
#include <math.h>
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
    for (int i = 1;
i < n;
i++) {
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
if (in->sun_set && out->count < GBUFFER_MAX_LIGHTS)
        out->lights[out->count++] = in->sun;
int n = (int)darr_len(in->items);
if (n <= 0) return;
static cull_entry scratch[4096];
int cand = 0;
for (int i = 0;
i < n && cand < (int)(sizeof scratch / sizeof *scratch);
for (int i = 0;
i < cand;
}
