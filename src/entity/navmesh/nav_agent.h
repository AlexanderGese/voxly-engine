#ifndef ENTITY_NAVMESH_NAV_AGENT_H
#define ENTITY_NAVMESH_NAV_AGENT_H

#include "nav_types.h"

// the agent profile a bake is shaped for. the default mesh is cut for the
// 2-tall mob we spawn everywhere (see NAV_AGENT_HEIGHT), but the step/jump
// rules and body height live here so a one-tall critter or a taller boss can
// ask for a mesh that fits them. the builder reads a profile instead of the
// raw NAV_* constants; the constants are just the default profile's values.
//
// keeping this separate from nav_filter on purpose: a filter masks an existing
// mesh at query time (cheap, shared), a profile changes what gets baked (a
// different mesh entirely). most mobs share the default and never touch this.

typedef struct {
    int height;      // body clearance in blocks above the floor
    int step_up;     // max climb that's still a walk
    int step_down;   // max controlled drop that's still a walk/drop link
    int jump_down;   // max fall a one-way drop link tolerates
} nav_agent;

// the stock profile, matching the NAV_* defaults. what you get if you bake
// without saying anything.
nav_agent nav_agent_default(void);

// a short squat profile (one-block clearance) for things that fit under a
// slab. baked meshes for these include tunnels the default mob can't enter.
nav_agent nav_agent_short(void);

// a tall profile (3 clearance, weaker jumps) for a big slow mob. fewer cells,
// no squeezing through gaps the default would.
nav_agent nav_agent_tall(void);

// sanity-clamp a profile so a hand-built one can't ask for nonsense (zero
// height, negative steps) and trip the builder. returns the clamped copy.
nav_agent nav_agent_sane(nav_agent a);

// do two profiles produce the same mesh? the cache uses this to decide whether
// a resident patch can be handed to a differently-shaped agent or has to be
// rebaked. only the bake-relevant fields count.
int nav_agent_eq(nav_agent a, nav_agent b);

#endif
