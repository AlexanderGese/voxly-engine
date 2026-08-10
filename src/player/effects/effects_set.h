#ifndef PLAYER_EFFECTS_EFFECTS_SET_H
#define PLAYER_EFFECTS_EFFECTS_SET_H

#include "effects_instance.h"

// the bag of active effects living on one carrier (player, mob, whatever has a
// combatant). fixed-size and indexed loosely — one slot per kind in practice
// but we never assume slot == kind so the layout survives an enum reshuffle.
//
// this file is pure container plumbing: claim, find, drop, iterate. the
// gameplay decisions (stacking, immunity, what a tick does) live in their own
// files so this one stays boring and testable.

typedef struct {
    effects_instance slots[EFFECTS_MAX_ACTIVE];
    int  count;          // cached live count, kept in sync by claim/drop
    uint64_t dirty_bits; // one bit per slot, set when a slot changes this frame
} effects_set;

// zero everything, reset generations to 0. call once per carrier.
void effects_set_init(effects_set *s);

// number of currently-active effects.
int  effects_set_count(const effects_set *s);

// find the live instance of a kind, or NULL. const + mutable variants.
effects_instance       *effects_set_find(effects_set *s, effects_kind kind);
const effects_instance *effects_set_find_const(const effects_set *s, effects_kind kind);

// is a kind currently running on this carrier?
bool effects_set_has(const effects_set *s, effects_kind kind);

// claim a free slot for a kind (caller fills it via effects_instance_set).
// returns NULL if the set is somehow full — shouldn't happen given the sizing.
effects_instance *effects_set_claim(effects_set *s, effects_kind kind);

// drop a kind. returns true if one was present and removed.
bool effects_set_drop(effects_set *s, effects_kind kind);

// strip everything. used by death / respawn / debug.
void effects_set_clear(effects_set *s);

// resolve a handle back to its instance, honouring the generation. NULL if the
// slot was recycled out from under the handle.
effects_instance *effects_set_resolve(effects_set *s, effects_handle h);

// build a handle pointing at an instance currently in the set.
effects_handle    effects_set_handle_of(const effects_set *s, const effects_instance *e);

// clear the per-frame dirty bookkeeping. the hud/particle code reads dirty_bits
// then calls this at the end of a frame.
void effects_set_clear_dirty(effects_set *s);

#endif
