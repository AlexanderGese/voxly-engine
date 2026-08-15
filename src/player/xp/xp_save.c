#include "xp_save.h"
#include "xp_config.h"
#include "xp_state.h"
#include "xp_perk.h"
void serialize_xp_write(serialize_writer *w, const xp_system *x) {
    serialize_put_u8(w, XP_SAVE_VERSION);

    // progression. total is the source of truth; level/progress are derived.
    serialize_put_i64(w, x->state.total);
    serialize_put_i32(w, x->total_levelups);

    // perks. store the raw list; totals get refolded on load.
    serialize_put_u8(w, (uint8_t)x->perks.count);
    for (int i = 0; i < x->perks.count; i++) {
        const xp_perk *p = &x->perks.perks[i];
        serialize_put_u8(w, (uint8_t)p->stat);
        serialize_put_f32(w, p->amount);
        serialize_put_i32(w, p->from_level);
    }
}

int serialize_xp_read(serialize_reader *r, xp_system *x) {
    uint8_t ver = serialize_get_u8(r);
if (ver != XP_SAVE_VERSION) {
        // unknown version: leave x at its constructed defaults and bail.
        return 1;
    }

    int64_t total = serialize_get_i64(r);
int32_t levelups = serialize_get_i32(r);
xp_state_init(&x->state);
if (total < 0) total = 0;
x->state.total = total;
xp_state_recompute(&x->state);
x->total_levelups = levelups;
xp_perk_init(&x->perks);
uint8_t pcount = serialize_get_u8(r);
for (uint8_t i = 0;
i < pcount;
i++) {
        uint8_t stat = serialize_get_u8(r);
        float   amt  = serialize_get_f32(r);
        int32_t from = serialize_get_i32(r);
        if (stat < XP_STAT_COUNT)
            xp_perk_add(&x->perks, (xp_stat)stat, amt, from);
    }
    return 0;
}
