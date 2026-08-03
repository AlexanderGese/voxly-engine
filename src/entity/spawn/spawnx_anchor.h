#ifndef ENTITY_SPAWN_SPAWNX_ANCHOR_H
#define ENTITY_SPAWN_SPAWNX_ANCHOR_H

#include "spawnx_types.h"
#include "../mob.h"

// persistence anchors. a mob with an anchor is exempt from the ambient despawn
// pass: command-spawned mobs, named/tamed mobs, event bosses you want to stick
// around. the despawn code in mspawn doesnt know about these, so the spawnx
// driver filters its despawn candidates through here first.
//
// also holds spawn-protection: a small set of points (player spawn, a bed)
// around which natural hostile spawning is suppressed. block spawners and the
// ambient loop both ask spawnx_anchor_protected() before committing a hostile.
//
// stored as a flat array indexed by a stable handle, not by registry slot,
// because registry slots get reused when a mob dies and we must not transfer an
// anchor to whoever lands in the recycled slot.

#define SPAWNX_ANCHOR_MAX     64
#define SPAWNX_PROTECT_MAX     4
#define SPAWNX_PROTECT_RADIUS 24.0f   // blocks of no-hostile-spawn around a point

typedef struct {
    uint32_t entity_id;   // 0 == free slot
    uint8_t  reason;      // spawnx_source that earned the anchor
    uint8_t  sticky;      // 1 => survives even chunk unload save/load
} spawnx_anchor;

typedef struct {
    spawnx_anchor anchor[SPAWNX_ANCHOR_MAX];
    int           anchor_count;
    vec3          protect[SPAWNX_PROTECT_MAX];
    int           protect_count;
} spawnx_anchor_store;

void spawnx_anchor_init(spawnx_anchor_store *s);

// latch an anchor onto an entity id. idempotent: re-anchoring an already
// anchored id just updates its fields. returns 1 on success, 0 if the store is
// full. sticky marks bosses/named mobs that should never be culled at all.
int  spawnx_anchor_add(spawnx_anchor_store *s, uint32_t entity_id,
                       spawnx_source reason, int sticky);

// drop an entity's anchor (it died, or got un-named). safe to call on a
// non-anchored id.
void spawnx_anchor_remove(spawnx_anchor_store *s, uint32_t entity_id);

// is this entity protected from despawn right now?
int  spawnx_anchor_is_held(const spawnx_anchor_store *s, uint32_t entity_id);

// prune anchors whose entity is no longer alive in the registry. call each
// cycle so dead mobs free their slots and dont pin a recycled id.
void spawnx_anchor_sweep(spawnx_anchor_store *s, const mob_registry *mr);

// spawn-protection points -----------------------------------------------------

// register a no-hostile-spawn point. replaces the oldest if full. typically the
// world spawn and the player's last bed.
void spawnx_anchor_set_protect(spawnx_anchor_store *s, int idx, vec3 p);

// would a hostile spawn at `pos` fall inside any protection bubble?
int  spawnx_anchor_protected(const spawnx_anchor_store *s, vec3 pos);

#endif
