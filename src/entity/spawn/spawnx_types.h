#ifndef ENTITY_SPAWN_SPAWNX_TYPES_H
#define ENTITY_SPAWN_SPAWNX_TYPES_H
#include <stdint.h>
#include "../../math/vec3.h"
#include "../entity.h"
// the *deliberate* spawning layer, sitting on top of the per-tick mspawn loop.
// mspawn handles the ambient "fill the loaded world with the right critters"
// job; spawnx handles everything that is placed or scripted: dungeon spawner
// blocks, persistence anchors that keep a named cow from despawning, scripted
// raids/sieges, and the regional cap accounting those need.
//
// kept on its own prefix so it never collides with mspawn_ while the two share
// the same entity_type / vec3 vocabulary. these are plain-data structs; nothing
// here touches the world or the registry, the driver in spawnx.c does that.
// reason a spawn was requested. mostly bookkeeping so the despawn pass and the
// debug overlay can tell a scripted siege zombie from an ambient one, and so
// anchors know which spawns to protect.
typedef enum {
    SPAWNX_SRC_AMBIENT = 0,  // came up through the mspawn loop, not ours
    SPAWNX_SRC_BLOCK,        // a monster-spawner block coughed it up
    SPAWNX_SRC_EVENT,        // part of a scripted wave (siege/raid/swarm)
    SPAWNX_SRC_COMMAND,      // forced in by debug / spawn egg, never despawns
    SPAWNX_SRC_COUNT
} spawnx_source;
// a request to put one mob into the world at a spot. producers (block spawners,
// event waves) fill these and hand them to the commit path, which does the
// final ground/headroom/collision check before it touches the registry. a
// rejected request is just dropped, so over-asking is fine and expected.
typedef struct {
    vec3          pos;       // desired feet position, voxel-centered
    entity_type   type;
    spawnx_source source;
    uint32_t      tag;       // owner handle: block id, event id, 0 for none
    int           persist;   // 1 => mark the result with a persistence anchor
} spawnx_request;
// result of trying to commit a request. the caller usually only cares whether
// it landed, but the entity id lets event waves track their own spawns and
typedef struct {
    int      placed;         // 1 if a mob entered the world
    uint32_t entity_id;      // valid only when placed
    vec3     pos;            // resolved position (may differ from requested)
} spawnx_result;
typedef struct { int rx, rz; } spawnx_region_key;
#define SPAWNX_REGION 64   // blocks per region edge. four chunks-ish.
static inline uint64_t spawnx_region_pack(spawnx_region_key k) {
    uint32_t ux = (uint32_t)(k.rx + 0x40000000);
    uint32_t uz = (uint32_t)(k.rz + 0x40000000);
    return ((uint64_t)ux << 32) | uz;
}

static inline spawnx_region_key spawnx_region_of(vec3 p) {
    // floor-divide, because truncation lies for negative coords.
    int bx = (int)p.x, bz = (int)p.z;
if (p.x < 0 && (float)bx != p.x) bx--;
if (p.z < 0 && (float)bz != p.z) bz--;
spawnx_region_key k;
k.rx = (bx >= 0) ? bx / SPAWNX_REGION
                     : -((-bx + SPAWNX_REGION - 1) / SPAWNX_REGION);
k.rz = (bz >= 0) ? bz / SPAWNX_REGION
                     : -((-bz + SPAWNX_REGION - 1) / SPAWNX_REGION);
return k;
}
#endif
