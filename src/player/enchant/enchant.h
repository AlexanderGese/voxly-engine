#ifndef PLAYER_ENCHANT_H
#define PLAYER_ENCHANT_H

// umbrella header for the enchanting subsystem. include this and you get the
// whole vocabulary: the registry, per-item sets, the table state machine, the
// weighted roller, gameplay modifiers, the anvil, costs and serialization.
//
// the dependency shape, roughly:
//
// types  <- registry <- set <- {roll, anvil, serial, modifier}
// types  <- power (needs world)
// {power, roll, set} <- table (needs world + rng)
// registry <- name
// cost is standalone
//
// nothing here pulls in gl or the renderer; the overlay that draws the table
// ui lives in render/ and only includes this for the data.

#include "enchant_types.h"
#include "enchant_registry.h"
#include "enchant_set.h"
#include "enchant_power.h"
#include "enchant_roll.h"
#include "enchant_table.h"
#include "enchant_modifier.h"
#include "enchant_anvil.h"
#include "enchant_cost.h"
#include "enchant_name.h"
#include "enchant_serial.h"
#include "enchant_session.h"

// one-call bootstrap for the whole subsystem. currently just primes the
// registry, but kept as a seam so future state (loaded tuning, caches) has a
// home. returns the number of enchant kinds available.
int enchant_init(void);

// has enchant_init run successfully?
int enchant_ready(void);

#endif
