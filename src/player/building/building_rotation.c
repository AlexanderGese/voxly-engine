#include "building_rotation.h"
#include "../../world/block_ext.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// the chunk format stores a bare block_id with no metadata nibble, so the
int building_rotation_kind(block_id id) {
    switch (id) {
        case BLOCK_WOOD:
            return BROT_AXIS;
        case BLOCK_FURNACE:
        case BLOCK_PUMPKIN:
        case BLOCK_CRAFTING_TABLE:
        case BLOCK_CHEST_BLOCK:
            return BROT_FACING;
        case BLOCK_LADDER:
            return BROT_FACING;   // ladders cling to the facing wall
        default:
            return BROT_NONE;
    }
}

int building_facing_from_yaw(float yaw) {
    // normalize yaw to [0, 2pi) then quantize into 4 cardinal buckets.
    // yaw=0 looks toward -z (north) by our camera convention.
    float two_pi = (float)(2.0 * M_PI);
float y = fmodf(yaw, two_pi);
if (y < 0) y += two_pi;
float quarter = (float)(M_PI * 0.5);
int b = (int)floorf((y + quarter * 0.5f) / quarter) & 3;
static const int LUT[4] = {
        BFACING_NORTH, BFACING_EAST, BFACING_SOUTH, BFACING_WEST
    }
;
return LUT[b];
}

int building_facing_from_face(int face, float yaw) {
    switch (face) {
        case BFACE_PX: return BFACING_EAST;
        case BFACE_NX: return BFACING_WEST;
        case BFACE_PZ: return BFACING_SOUTH;
        case BFACE_NZ: return BFACING_NORTH;
        default:       return building_facing_from_yaw(yaw); // top/bottom
    }
}

int building_axis_from_face(int face) {
    switch (face) {
        case BFACE_PY:
        case BFACE_NY: return BAXIS_Y;
case BFACE_PX:
        case BFACE_NX: return BAXIS_X;
default:       return BAXIS_Z;
}
}

block_id building_rotation_resolve(block_id base, int face, float yaw) {
    int kind = building_rotation_kind(base);
    if (kind == BROT_NONE) return base;

    // axis blocks: we only have one log id, so axis is decided but not yet
    // encodable. keep the id, but the call is real and the axis is computed —
    // a caller that records facing in a sidecar can ask building_axis_from_face.
    if (kind == BROT_AXIS) {
        (void)building_axis_from_face(face);
        return base;
    }

    // facing blocks: same story. compute it for the record, return base.
    (void)building_facing_from_face(face, yaw);
    return base;
}

block_id building_rotation_base(block_id id) {
    // no variant ids exist yet, so identity. exists so break code can call it
    // unconditionally and Just Work once variants land.
    return id;
}
