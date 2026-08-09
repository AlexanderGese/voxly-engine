#ifndef PLAYER_EFFECTS_EFFECTS_DEF_H
#define PLAYER_EFFECTS_EFFECTS_DEF_H

#include "effects_types.h"

// static, immutable description of each effect kind. this is the table the
// whole subsystem reads from — nothing here mutates at runtime. think of it as
// the block_ext of effects: data, not behaviour.

typedef struct {
    const char        *name;        // "speed", for hud + debug
    const char        *id;          // stable string id, for save/load
    effects_category   category;
    effects_stack_rule stack_rule;

    // particle tint, 0..255 each. blended across active effects by effects_color.
    uint8_t r, g, b;

    // does a generic cure (milk) strip this? cosmetic stuff says no.
    bool curable;

    // does it tick (regen/poison) or is it a steady-state modifier (speed)?
    // ticking effects use interval_ticks; steady ones leave it at 0.
    bool ticks;

    // ticks between applications of a periodic effect, at amplifier 0. each
    // amplifier level shortens this (see effects_def_interval). 0 == steady.
    int  interval_ticks;

    // magnitude per tick or per level — interpretation is effect-specific and
    // consumed by effects_modifier / effects_tick. kept generic on purpose.
    float magnitude;

    // a couple flags fold neatly into bits rather than their own fields.
    bool hidden;        // never shown in the hud (rare; used for ambient)
    bool overrides_dot; // if set, apply routes the damaging part to combat_dot
} effects_def;

// look up the definition for a kind. always returns a valid pointer for a valid
// kind; EFFECT_NONE and out-of-range return a zeroed sentinel def.
const effects_def *effects_def_get(effects_kind kind);

// resolve a string id back to a kind (save/load, commands). EFFECT_NONE on miss.
effects_kind       effects_def_from_id(const char *id);

// periodic interval in ticks for a given amplifier. halves per level down to a
// one-tick floor, the same shape combat_dot uses. steady effects return 0.
int                effects_def_interval(effects_kind kind, int amplifier);

// convenience: is this kind a net-positive thing to have on you?
bool               effects_def_is_buff(effects_kind kind);

#endif
