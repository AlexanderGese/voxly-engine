#ifndef ENTITY_BEHAVIOR_PREFAB_H
#define ENTITY_BEHAVIOR_PREFAB_H

#include "behavior_tree.h"

// canned trees for the mob kinds we ship. the game asks for one of these when
// it spawns a mob, gets back a fully built tree, sets a few blackboard tuning
// values, and ticks it each frame. saves every mob_* file from hand-rolling
// the same selector/sequence shapes.

// hostile: pick attack > chase > wander, with a hurt override on top.
// roughly:
// selector
// sequence  [is_hurt? -> face target -> attack]   (lash out when hit)
// sequence  [in_reach? -> attack]
// sequence  [in_sight? -> chase]
// wander
void behavior_prefab_hostile(behavior_tree *out);

// passive prey: flee when something is close/hurting us, otherwise wander.
// selector
// sequence  [is_hurt? -> flee]
// sequence  [target_in_sight? -> flee]
// wander
void behavior_prefab_passive(behavior_tree *out);

// skittish: same as passive but flees on mere sight from further away and
// never stands still for long. used by chickens/rabbits.
void behavior_prefab_skittish(behavior_tree *out);

// fill a tree's blackboard with default tuning for a mob kind. call after
// building so per-kind speeds/ranges are set without the caller knowing them.
void behavior_prefab_defaults(behavior_tree *t, int mob_kind);

#endif
