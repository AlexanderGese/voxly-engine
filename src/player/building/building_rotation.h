#ifndef PLAYER_BUILDING_ROTATION_H
#define PLAYER_BUILDING_ROTATION_H

#include "building_types.h"
#include "../../math/vec3.h"

// rotation resolution for placed blocks. we don't have per-block metadata in
// the chunk format yet (blocks[] is just a block_id), so for blocks that come
// in pre-rotated variants we resolve to the right *variant id* at place time.
// for everything else this is a no-op that returns the id unchanged.

// a block's rotation capability. determined from its id.
enum {
    BROT_NONE = 0,   // not rotatable
    BROT_AXIS,       // pillar, 3 states (logs)
    BROT_FACING,     // 4 horizontal facings (furnace, pumpkin)
    BROT_FULL        // facing incl. up/down (not used much, but logs+stairs)
};

int building_rotation_kind(block_id id);

// horizontal facing the player is looking *away from* — i.e. the facing a
// block placed in front of you should adopt so its "front" looks back at you.
int  building_facing_from_yaw(float yaw);

// the facing implied by which face was clicked. e.g. clicking the +x face of
// a neighbor means the new block faces east. vertical faces fall back to yaw.
int  building_facing_from_face(int face, float yaw);

// pillar axis from the face that was clicked. clicking a top/bottom face
// stands the log upright (Y), side faces lay it along X or Z.
int  building_axis_from_face(int face);

// resolve a base block id + orientation into the concrete id to store.
// `face` is the surface clicked, `yaw` the player's heading in radians.
// for non-rotatable blocks this just echoes `base`.
block_id building_rotation_resolve(block_id base, int face, float yaw);

// inverse-ish: recover the canonical/base id from a placed (possibly rotated)
// variant. used by break-drops so a sideways log still drops a normal log.
block_id building_rotation_base(block_id id);

#endif
