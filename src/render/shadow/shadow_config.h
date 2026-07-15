#ifndef RENDER_SHADOW_CONFIG_H
#define RENDER_SHADOW_CONFIG_H

// cascaded shadow map tunables. all the knobs live here so i dont have to
// go hunting through six files when the shadows look like garbage again.

// number of cascades. 4 is the sweet spot for our render distance. going
// to 5 barely helped and cost another depth pass.
#define SHADOW_CASCADE_COUNT     4

// per-cascade depth map resolution (square). same for every cascade — i
// tried shrinking the far ones and the popping wasnt worth the bytes saved.
#define SHADOW_MAP_SIZE          2048

// how far out we bother casting shadows, in world units. anything past this
// just samples the last cascade with no shadow. tuned against RENDER_DISTANCE.
#define SHADOW_MAX_DISTANCE      180.0f

// near plane for the shadow frustum slicing. we dont start at the camera
// znear because the first cascade would be microscopic.
#define SHADOW_NEAR_OFFSET       0.5f

// blend between uniform and logarithmic split. 0 = pure uniform (even slices),
// 1 = pure log (tight near, loose far). 0.75 looks right for voxels.
#define SHADOW_SPLIT_LAMBDA      0.75f

// pad the cascade ortho box outward so geometry just behind the slice still
// casts into it. in texels-ish world units.
#define SHADOW_BOUNDS_PAD        2.5f

// pull the light position back along -dir by this much so tall things behind
// the view frustum (trees, cliffs) still make it into the depth map.
#define SHADOW_LIGHT_PULLBACK    64.0f

// depth bias. constant + slope-scaled, applied in the depth pass via
// glPolygonOffset. voxel faces are axis aligned so acne is mostly on the
// slopes of the terrain mesh.
#define SHADOW_DEPTH_BIAS_CONST  1.25f
#define SHADOW_DEPTH_BIAS_SLOPE  2.75f

// normal-offset bias, in world units, scaled per cascade. pushes the sample
// point off the surface to kill the remaining acne without ballooning peter
// panning.
#define SHADOW_NORMAL_BIAS       0.04f

// pcf kernel radius in texels. 2 -> 5x5 taps. 3 gets soft but expensive.
#define SHADOW_PCF_RADIUS        2

// rotate the pcf kernel per-fragment to trade banding for noise. cheap.
#define SHADOW_PCF_JITTER        1

// fade shadows out over this fraction of SHADOW_MAX_DISTANCE so the edge of
// the shadowed region isnt a hard line.
#define SHADOW_FADE_BAND         0.12f

#endif
