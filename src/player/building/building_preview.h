#ifndef PLAYER_BUILDING_PREVIEW_H
#define PLAYER_BUILDING_PREVIEW_H
#include "building_types.h"
#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include "../../world/world.h"
// the crosshair outline + ghost block. the renderer reads this every frame to
// draw the white wireframe around the looked-at block and (optionally) a faint
// ghost of what would be placed. no gl in here — just the geometry and a bit
// of state so the outline doesn't flicker on single-frame raycast misses.
typedef struct {
    int   show_outline;   // draw the selection box?
    aabb  outline;        // box around the hit block (slightly inflated)
    int   outline_face;   // which face is highlighted (BFACE_*), -1 = none

    int   show_ghost;     // draw the would-place ghost?
    aabb  ghost;          // box at the placement cell
    block_id ghost_id;    // block that would be placed
    int   ghost_valid;    // BPLACE_OK -> tint green-ish, else red-ish

    // hysteresis: keep the last good outline alive for a few frames so a
    // momentary miss (player flicks across an edge) doesn't strobe it.
    int   grace;
} building_preview;
void building_preview_init(building_preview *pv);
// recompute from the current target. `place_id` is what the player would place
// (0/air to disable the ghost). `place_ok` is the BPLACE_* verdict for tinting
// — pass BPLACE_OK if you don't want to bother validating for the preview.
// `grace_frames` is how long to keep a stale outline (e.g. 3).
void building_preview_update(building_preview *pv, const building_target *t,
                             block_id place_id, int place_ok, int grace_frames);
// the standard slightly-inflated selection box for a block cell. exposed so
// the renderer can reuse it for other highlights.
aabb building_preview_box(int x, int y, int z, float inflate);
// the 8 corners of an aabb written into out[8], for a line-list wire cube.
void building_preview_corners(aabb box, vec3 out[8]);
// the 24 vertex indices (12 edges * 2) of a wire cube, into out[24]. pairs
// reference building_preview_corners output. static topology, but handy.
void building_preview_edges(int out[24]);
#endif
