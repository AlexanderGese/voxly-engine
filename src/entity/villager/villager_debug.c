#include "villager_debug.h"
#include "villager_def.h"
#include "../../util/darray.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static const char *act_names[VILLAGER_ACT_COUNT] = {
    "sleep", "work", "gather", "wander", "panic",
};

const char *villager_debug_activity_name(villager_activity a) {
    if (a < 0 || a >= VILLAGER_ACT_COUNT) return "?";
    return act_names[a];
}

int villager_debug_describe(const villager *v, char *buf, int cap) {
    if (cap <= 0) return 0;
    // e.g. "#42 farmer L2 work hp20/20 @ (12,64,-8) baby:no"
    int n = snprintf(buf, (size_t)cap,
                     "#%u %s L%d %s hp%d/%d @ (%d,%d,%d) baby:%s",
                     v->id,
                     villager_def_name(v->prof),
                     v->level,
                     villager_debug_activity_name(v->activity),
                     v->hp, v->max_hp,
                     (int)v->pos.x, (int)v->pos.y, (int)v->pos.z,
                     v->is_baby ? "yes" : "no");
    if (n < 0) { buf[0] = '\0'; return 0; }
    if (n >= cap) n = cap - 1;   // snprintf truncated; report what fit
    return n;
}

void villager_debug_collect(const villager_manager *m, villager_debug_stats *out) {
    memset(out, 0, sizeof *out);
    for (size_t i = 0; i < darr_len(m->vills); i++) {
        const villager *v = &m->vills[i];
        if (!villager_is_alive(v)) continue;
        out->total++;
        if (v->is_baby) out->babies++;
        if (v->prof != VILLAGER_PROF_UNEMPLOYED &&
            v->prof != VILLAGER_PROF_NITWIT && !v->is_baby)
            out->employed++;
        if (v->activity == VILLAGER_ACT_SLEEP) out->sleeping++;
        if (v->activity == VILLAGER_ACT_PANIC) out->panicking++;
        if (v->prof >= 0 && v->prof < VILLAGER_PROF_COUNT)
            out->by_prof[v->prof]++;
    }
}
