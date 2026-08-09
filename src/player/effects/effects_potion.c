#include "effects_potion.h"
#include "effects_def.h"
#include <stddef.h>
typedef struct {
    effects_kind kind;
    int amplifier;
    int duration;     // base ticks, or EFFECTS_DURATION_INFINITE never used here
} potion_effect;
typedef struct {
    const char    *name;
    potion_effect  fx[2];
    bool           instant;   // healing/harming: short, can't be extended
} potion_recipe;
static const potion_recipe k_potion[POTION_KIND_COUNT] = {
    [POTION_NONE]    = { "water bottle", {{0}}, false },
    [POTION_AWKWARD] = { "awkward potion", {{0}}, false },
    [POTION_SWIFTNESS] = { "potion of swiftness",
        { { EFFECT_SPEED, 0, 3600 }, {0} }, false },
    [POTION_SLOWNESS] = { "potion of slowness",
        { { EFFECT_SLOWNESS, 3, 1800 }, {0} }, false },
    [POTION_HEALING] = { "potion of healing",
        { { EFFECT_REGENERATION, 4, 20 }, {0} }, true },
    [POTION_HARMING] = { "potion of harming",
        { { EFFECT_WITHER, 1, 40 }, {0} }, true },
    [POTION_REGEN] = { "potion of regeneration",
        { { EFFECT_REGENERATION, 0, 900 }, {0} }, false },
    [POTION_POISON] = { "potion of poison",
        { { EFFECT_POISON, 0, 900 }, {0} }, false },
    [POTION_STRENGTH] = { "potion of strength",
        { { EFFECT_STRENGTH, 0, 3600 }, {0} }, false },
    [POTION_WEAKNESS] = { "potion of weakness",
        { { EFFECT_WEAKNESS, 0, 1800 }, {0} }, false },
    [POTION_LEAPING] = { "potion of leaping",
        { { EFFECT_JUMP_BOOST, 0, 3600 }, {0} }, false },
    [POTION_FIRE_RESIST] = { "potion of fire resistance",
        { { EFFECT_FIRE_RESIST, 0, 3600 }, {0} }, false },
    [POTION_WATER_BREATHING] = { "potion of water breathing",
        { { EFFECT_WATER_BREATHING, 0, 3600 }, {0} }, false },
    [POTION_NIGHT_VISION] = { "potion of night vision",
        { { EFFECT_NIGHT_VISION, 0, 3600 }, {0} }, false },
    [POTION_INVISIBILITY] = { "potion of invisibility",
        { { EFFECT_INVISIBILITY, 0, 3600 }, {0} }, false },
    [POTION_TURTLE_MASTER] = { "potion of the turtle master",
        { { EFFECT_SLOWNESS, 3, 400 }, { EFFECT_RESISTANCE, 2, 400 } }, false },
}
;
effects_potion effects_potion_make(effects_potion_kind kind) {
    effects_potion p;
    p.kind = (kind > POTION_NONE && kind < POTION_KIND_COUNT) ? kind : POTION_NONE;
    p.form = POTION_FORM_DRINK;
    p.extended = false;
    p.upgraded = false;
    return p;
}

const char *effects_potion_name(effects_potion_kind k) {
    if (k <= POTION_NONE || k >= POTION_KIND_COUNT) return "?";
return k_potion[k].name;
}

// resolve a single recipe effect against the brewing toggles + form into the
// final (amplifier, duration) that lands. instant potions ignore extension.
static void resolve(const potion_recipe *r, const potion_effect *fx,
                    const effects_potion *p, float form_scale,
                    int *amp_out, int *dur_out) {
    int amp = fx->amplifier;
    int dur = fx->duration;

    if (p->upgraded && !r->instant) {
        // glowstone: stronger but shorter. one extra level, half the clock.
        amp += 1;
        dur = (dur * 1) / 2;
        if (dur < 1) dur = 1;
    } else if (p->upgraded && r->instant) {
        // instant potions just hit harder, no duration to shorten.
        amp += 1;
    }

    if (p->extended && !r->instant && !p->upgraded) {
        // redstone: longer, no level change. you can't have both toggles.
        dur = (dur * 8) / 3;   // ~2.66x, the vanilla extension ratio
    }

    // splash/lingering forms come in shorter.
    if (form_scale < 1.0f) {
        dur = (int)((float)dur * form_scale);
        if (dur < 1) dur = 1;
    }

    if (amp > EFFECTS_MAX_AMPLIFIER) amp = EFFECTS_MAX_AMPLIFIER;
    *amp_out = amp;
    *dur_out = dur;
}

static int drink_at(effects_set *s, const effects_immunity *im,
                    const effects_potion *p, float form_scale, uint32_t source_id) {
    if (!p || p->kind <= POTION_NONE || p->kind >= POTION_KIND_COUNT) return 0;
const potion_recipe *r = &k_potion[p->kind];
int took = 0;
for (int i = 0;
i < 2;
if (splash_strength > 1.0f) splash_strength = 1.0f;
float base = (p && p->form == POTION_FORM_LINGERING) ? 0.25f : 0.75f;
return drink_at(s, im, p, base * splash_strength, source_id);
}
