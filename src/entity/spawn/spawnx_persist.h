#ifndef ENTITY_SPAWN_SPAWNX_PERSIST_H
#define ENTITY_SPAWN_SPAWNX_PERSIST_H

#include "spawnx_anchor.h"
#include "mspawn_rand.h"
#include "../mob.h"

// the despawn pass, but anchor-aware. mspawn_despawn culls everything past its
// radii; that would happily delete a tamed wolf or an event boss. this is the
// spawnx replacement the driver runs instead: identical two-band logic (hard
// radius vanish, soft radius slow roll) but it consults the anchor store first
// and never touches a held mob. sticky anchors are also immune to the very
// occasional "too far from any player" forced cull.

#define SPAWNX_DESPAWN_HARD   80.0f
#define SPAWNX_DESPAWN_SOFT   48.0f
#define SPAWNX_DESPAWN_CHANCE  0.02f

// per-mob linger memory, parallel to the registry by slot index, same shape as
// mspawn's so it can drop in. lives in the driver.
typedef struct {
    float linger[MAX_MOBS];
} spawnx_despawn_state;

void spawnx_despawn_init(spawnx_despawn_state *st);

// run one anchor-aware despawn pass. removes far mobs that arent anchored, rolls
// the soft band, advances linger timers. returns how many it culled. when a mob
// is removed its anchor (if any, e.g. it wandered off and the anchor lapsed) is
// swept by the caller separately.
int  spawnx_despawn_tick(spawnx_despawn_state *st, mob_registry *mr,
                         const spawnx_anchor_store *anchors,
                         vec3 player_pos, mspawn_rng *r, float dt);

#endif
