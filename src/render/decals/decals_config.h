#ifndef RENDER_DECALS_CONFIG_H
#define RENDER_DECALS_CONFIG_H

// deferred decal tunables. all the magic numbers for the subsystem live here
// so the rest of the files can stay readable.h.

// hard cap on live decals. each one is a projector box stamped into the
// g-buffer after the opaque pass. 1024 is plenty for a voxel world — most of
// what we stamp is footprints, blast scorch, and the odd bullet hole.
#define DECALS_MAX               1024

// the decal atlas is the same layout idea as the block atlas: a grid of square
// tiles. one tile = one decal image (color in rgb, height/coverage in a). we
// keep a parallel grid of normal tiles so a decal can perturb the surface.
#define DECALS_ATLAS_TILES_X      8
#define DECALS_ATLAS_TILES_Y      8
#define DECALS_ATLAS_MAX_REGIONS  (DECALS_ATLAS_TILES_X * DECALS_ATLAS_TILES_Y)

// projector defaults. a decal projects along its local -Z, clamped to surfaces
// whose normal is within this cone of the projector forward. beyond it the
// stamp would smear across a near-perpendicular wall, which looks awful.
#define DECALS_DEFAULT_ANGLE_FADE   0.35f   // cos-space, ~70 deg cone
#define DECALS_MIN_HALF_EXTENT       0.02f  // refuse to spawn degenerate boxes
#define DECALS_MAX_HALF_EXTENT      32.0f

// fade timing. a decal eases in, lives flat, then eases out before it dies.
// times are seconds. ttl <= 0 means "immortal", skip the death fade.
#define DECALS_DEFAULT_FADE_IN       0.15f
#define DECALS_DEFAULT_FADE_OUT      0.75f

// normal-blend strength clamp. 1 = fully replace surface normal with the
// decal's, 0 = ignore it. we never quite hit 1, tangent basis is approximate.
#define DECALS_NORMAL_BLEND_MAX      0.85f

// soft depth rejection: if the reconstructed surface is more than this many
// world units off the projector's near plane we treat it as "not the wall we
// meant" and discard the fragment. stops decals leaking onto far geometry.
#define DECALS_DEPTH_SLOP            0.06f

// how many decals we draw per instanced batch. the instance vbo is sized to
// this; the pass loops if the visible set is larger.
#define DECALS_BATCH_MAX            256

#endif
