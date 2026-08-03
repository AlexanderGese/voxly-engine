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
int  spawnx_anchor_add(spawnx_anchor_store *s, uint32_t entity_id,
                       spawnx_source reason, int sticky);
void spawnx_anchor_remove(spawnx_anchor_store *s, uint32_t entity_id);
int  spawnx_anchor_is_held(const spawnx_anchor_store *s, uint32_t entity_id);
void spawnx_anchor_sweep(spawnx_anchor_store *s, const mob_registry *mr);
void spawnx_anchor_set_protect(spawnx_anchor_store *s, int idx, vec3 p);
int  spawnx_anchor_protected(const spawnx_anchor_store *s, vec3 pos);
#endif
