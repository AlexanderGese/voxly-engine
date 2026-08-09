#ifndef PLAYER_EFFECTS_EFFECTS_H
#define PLAYER_EFFECTS_EFFECTS_H

// umbrella include for the status-effect subsystem. pull this and you get the
// whole thing; the rest of the engine should only ever need effects_controller
// plus the modifier struct, but the lower headers are here for the few callers
// (tests, save/load, the brewing ui) that reach past the façade.
//
// layering, bottom to top:
// types    -> the vocabulary (kinds, categories, handles)
// def      -> the static per-kind data table
// instance -> one live effect + the stacking math
// set      -> the fixed bag of instances on a carrier
// curative -> immunities + milk/antidote
// apply    -> the one front door for granting effects
// tick     -> the heartbeat that advances + fires periodic effects
// modifier -> derived attribute snapshot the game multiplies in
// potion   -> item -> effect-list bridge + brewing toggles
// color    -> particle swirl blending
// hud      -> sorted display list for the ui
// event    -> ring log of transitions for toasts/debug
// controller -> the glue that wires set+events+immunity together

#include "effects_types.h"
#include "effects_def.h"
#include "effects_instance.h"
#include "effects_set.h"
#include "effects_curative.h"
#include "effects_apply.h"
#include "effects_tick.h"
#include "effects_modifier.h"
#include "effects_potion.h"
#include "effects_color.h"
#include "effects_hud.h"
#include "effects_event.h"
#include "effects_controller.h"

#endif
