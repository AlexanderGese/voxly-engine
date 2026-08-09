#ifndef PLAYER_EFFECTS_EFFECTS_CURATIVE_H
#define PLAYER_EFFECTS_EFFECTS_CURATIVE_H

#include "effects_set.h"

// cures and immunities. the "take this off me" half of the subsystem, plus the
// rules for what simply refuses to land in the first place.
//
// kept apart from peffects_apply so the apply path can ask "may this land?"
// without dragging in the whole curing api, and vice versa.

// an immunity mask is a bitset over effect kinds. a fire-resist potion makes you
// immune to nothing here — immunities are the harder rule, e.g. an undead mob
// can't be poisoned. EFFECTS_MAX_ACTIVE < 64 so one uint64 covers it.
typedef struct {
    uint64_t bits;
} effects_immunity;

void effects_immunity_clear(effects_immunity *im);
void effects_immunity_add(effects_immunity *im, effects_kind kind);
void effects_immunity_remove(effects_immunity *im, effects_kind kind);
bool effects_immunity_has(const effects_immunity *im, effects_kind kind);

// preset: the classic undead profile — immune to poison/regen, hurt by healing.
// returned by value so callers can tweak before use.
effects_immunity effects_immunity_undead(void);

// strip every curable effect (milk bucket / honey). cosmetic effects flagged
// uncurable in the def table survive. returns how many were removed.
int  effects_curative_milk(effects_set *s);

// strip only harmful effects, keep buffs (a gentler antidote). returns count.
int  effects_curative_antidote(effects_set *s);

// strip exactly one kind regardless of curability (a targeted cure / command).
bool effects_curative_remove(effects_set *s, effects_kind kind);

// would an application of `kind` be allowed given an immunity mask? also blocks
// the obviously-nonsensical (EFFECT_NONE, out of range).
bool effects_curative_admits(const effects_immunity *im, effects_kind kind);

#endif
