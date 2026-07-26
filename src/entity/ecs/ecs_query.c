#include "ecs_query.h"

#include <stdarg.h>

// pick the required component whose store has the fewest entries -- that's the
// cheapest one to drive iteration from. if the require mask is empty we fall
// back to the transform store (basically everything has one) so we at least
// iterate *something* concrete rather than the whole id space.
static const ecs_store *pick_lead(ecs_world *w, ecs_signature require,
                                  ecs_cmp *out_cmp) {
    const ecs_store *best = NULL;
    uint32_t best_n = 0xFFFFFFFFu;
    ecs_cmp  best_c = ECS_CMP_TRANSFORM;

    for (int c = 0; c < ECS_CMP_COUNT; c++) {
        if (!ecs_sig_has(require, (ecs_component_id)c)) continue;
        uint32_t n = ecs_store_count(&w->stores[c]);
        if (n < best_n) { best_n = n; best = &w->stores[c]; best_c = (ecs_cmp)c; }
    }
    if (!best) {
        best   = &w->stores[ECS_CMP_TRANSFORM];
        best_c = ECS_CMP_TRANSFORM;
    }
    *out_cmp = best_c;
    return best;
}

void ecs_query_begin(ecs_query *q, ecs_world *w,
                     ecs_signature require, ecs_signature exclude) {
    q->w       = w;
    q->require = require;
    q->exclude = exclude;
    q->cursor  = 0;
    q->e       = ECS_NULL;

    ecs_cmp lead_c;
    q->lead = pick_lead(w, require, &lead_c);

    // cache which components the caller will likely fetch (everything required
    // except the lead, which it already iterates). purely a perf nicety.
    q->fetch_n = 0;
    for (int c = 0; c < ECS_CMP_COUNT && q->fetch_n < ECS_QUERY_MAX_GET; c++) {
        if (ecs_sig_has(require, (ecs_component_id)c) && (ecs_cmp)c != lead_c)
            q->fetch[q->fetch_n++] = (ecs_cmp)c;
    }
}

int ecs_query_next(ecs_query *q) {
    uint32_t n = ecs_store_count(q->lead);
    while (q->cursor < n) {
        ecs_entity e = ecs_store_entity_at(q->lead, q->cursor);
        q->cursor++;

        // the lead store can briefly hold entities the pool already killed if a
        // remove raced a destroy; re-validate against the pool to be safe.
        if (!ecs_pool_alive(&q->w->pool, e)) continue;

        ecs_signature sig = ecs_pool_sig(&q->w->pool, e);
        if (!ecs_sig_match(sig, q->require)) continue;
        if (sig & q->exclude)                continue;   // any excluded bit set

        q->e = e;
        return 1;
    }
    q->e = ECS_NULL;
    return 0;
}

void *ecs_query_get(ecs_query *q, ecs_cmp c) {
    return ecs_store_get(&q->w->stores[c], q->e);
}

uint32_t ecs_query_count(ecs_world *w, ecs_signature require,
                         ecs_signature exclude) {
    ecs_query q;
    ecs_query_begin(&q, w, require, exclude);
    uint32_t n = 0;
    while (ecs_query_next(&q)) n++;
    return n;
}

ecs_signature ecs_with(int first, ...) {
    ecs_signature s = 0;
    if (first < 0) return s;
    s = ecs_sig_set(s, (ecs_component_id)first);

    va_list ap;
    va_start(ap, first);
    for (;;) {
        int c = va_arg(ap, int);
        if (c < 0) break;                 // -1 sentinel terminates the list
        s = ecs_sig_set(s, (ecs_component_id)c);
    }
    va_end(ap);
    return s;
}
