#ifndef ENTITY_SPAWN_SPAWNX_EVENT_H
#define ENTITY_SPAWN_SPAWNX_EVENT_H

#include "spawnx_types.h"
#include "spawnx_weight.h"
#include "spawnx_region.h"
#include "mspawn_rand.h"
#include "../mob.h"
#include "../../world/world.h"

// scripted spawn events: a siege, a raid, a swarm. unlike the ambient loop and
// the block spawner, an event has a goal (spawn N mobs over M waves around a
// focus point) and a lifetime. one event drives a sequence of waves; each wave
// drains a small spawn-cost budget from a biome roster and scatters its mobs in
// a ring around the focus so they come from "outside" and converge.
//
// the event owns no mobs after they spawn; it just tags them via the commit
// path's source so the rest of the system can tell siege mobs from drifters.

typedef enum {
    SPAWNX_EV_NONE = 0,
    SPAWNX_EV_RUNNING,
    SPAWNX_EV_DONE,
} spawnx_event_phase;

typedef struct {
    spawnx_event_phase phase;
    vec3        focus;          // ring center, usually the player or a village
    biome_id    biome;          // roster to draw from
    mspawn_rng  rng;

    int         waves_total;    // how many waves the event runs
    int         waves_done;
    int         wave_budget;    // spawn-cost points per wave

    float       wave_timer;     // counts down to the next wave
    float       wave_gap;       // seconds between waves

    float       ring_min;       // inner/outer ring radius for placement
    float       ring_max;

    int         spawned;        // running total, for the hud / completion
    uint32_t    id;             // event handle, tags its spawns
} spawnx_event;

// configure a siege-style event around `focus`. waves/budget/gap pick the feel:
// a swarm is many small fast waves, a boss raid is few big slow ones. seeds its
// own rng so a replay reproduces the wave composition.
void spawnx_event_begin(spawnx_event *ev, vec3 focus, biome_id biome,
                        int waves, int wave_budget, float wave_gap,
                        uint32_t id, unsigned seed);

// step the event. fires a wave when its timer elapses, placing mobs in the ring
// through the commit path and the region cap. returns mobs spawned this tick.
// flips to DONE after the last wave; safe to keep ticking a DONE event (no-op).
int  spawnx_event_tick(spawnx_event *ev, world *w, mob_registry *mr,
                       spawnx_region_map *rm, float dt);

// has the event finished all its waves?
int  spawnx_event_finished(const spawnx_event *ev);

#endif
