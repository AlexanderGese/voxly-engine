#ifndef ENTITY_SPAWN_SPAWNX_SERIALIZE_H
#define ENTITY_SPAWN_SPAWNX_SERIALIZE_H

#include <stdint.h>
#include <stddef.h>
#include "spawnx.h"

// persistence for the deliberate-spawning state. the ambient mspawn population
// is disposable (it just refills on load), but the placed bits are not: a player
// who cleared half a dungeon expects those spawner cages gone, and a named pet's
// anchor must survive a save or it would despawn on the first reload. so we blob
// the spawner cages and the sticky anchors into a sidecar, same shape as the
// colorlight serializer.
//
// note we only persist *sticky* anchors and the cages. ordinary command/event
// anchors are tied to mobs that the entity save handles on its own; re-anchoring
// those is the entity loader's job, not ours.
//
// blob layout (all little-endian):
// u32 magic   ('SPWX')
// u16 version
// u16 spawner_count
// u16 anchor_count
// spawner_count * { i32 wx, i32 wy, i32 wz, u8 type, u8 biome }
// anchor_count  * { u32 entity_id, u8 reason }

#define SPAWNX_SAVE_MAGIC   0x58575053u  /* 'SPWX' little-endian */
#define SPAWNX_SAVE_VERSION 1

// worst-case encoded size for sizing a scratch buffer.
size_t spawnx_serialize_max_bytes(void);

// encode the persistent slice of `sx` into dst (>= max_bytes). returns bytes
// written, or 0 on a null arg.
size_t spawnx_serialize_encode(const spawnx *sx, uint8_t *dst, size_t cap);

// decode a blob back into `sx`. spawner cages are recreated (rng reseeded from
// their position + the world seed already in sx), sticky anchors are re-latched.
// existing live spawners/anchors are cleared first. returns bytes consumed, or 0
// on a malformed / wrong-magic blob.
size_t spawnx_serialize_decode(spawnx *sx, const uint8_t *src, size_t len);

#endif
