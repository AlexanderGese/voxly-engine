#ifndef RENDER_DECALS_FADE_H
#define RENDER_DECALS_FADE_H

#include "decals_types.h"

// the fade state machine. each decal walks SPAWNING -> STABLE -> DYING -> DEAD,
// and the alpha it presents to the renderer is a function of which phase it's
// in and how far through that phase it is. this is the only thing that mutates
// a decal's phase/alpha, so the pool can stay dumb storage.

// advance one decal by dt seconds. updates phase, life_elapsed and alpha.
// returns 1 if the decal is still alive afterwards, 0 if it just hit DEAD and
// the pool should reclaim the slot.
int decals_fade_tick(decals_decal *d, float dt);

// resolve the alpha for a given phase+progress without touching state. exposed
// for the cull pass, which wants the would-be alpha to early-skip fully faded
// decals without stepping them. progress is [0,1] within the phase.
float decals_fade_curve(decals_phase phase, float progress,
                        float fade_in, float fade_out);

// force a decal into its dying phase early (e.g. an explosion oversaturates an
// area and we want to retire old splats). no-op if already dying/dead.
void decals_fade_kill(decals_decal *d);

// (re)set the fade parameters and drop the decal back to the spawning ramp.
// used by the pool when it recycles a slot.
void decals_fade_reset(decals_decal *d, float life_total,
                       float fade_in, float fade_out);

#endif
