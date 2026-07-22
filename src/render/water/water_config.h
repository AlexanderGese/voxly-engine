#ifndef RENDER_WATER_WATER_CONFIG_H
#define RENDER_WATER_WATER_CONFIG_H

// tunables for the water surface renderer. kept apart from the global
// config.h so i can fiddle with these without recompiling half the tree.
// none of these are sacred, i landed on them by squinting at the lake.

// the reflection/refraction render targets dont need full resolution.
// half res looks basically identical once the surface ripples hide it.
#define WATER_RTT_DOWNSCALE     2

// minimum render target size so we never make a 0x0 fbo on a tiny window
#define WATER_RTT_MIN_DIM       16

// the surface plane sits a hair below the block top so the clip plane
// doesnt z-fight the actual water geometry when we render the world twice
#define WATER_PLANE_BIAS        0.02f

// how far above/below the plane we still consider geometry "at the surface".
// used when softening the shoreline against the depth buffer.
#define WATER_EDGE_SOFTNESS     0.6f

// fresnel. schlick approximation, this is the reflectance at normal incidence
// for an air->water boundary (ior ~1.33). gives that "water is mirror at
// grazing angles, glassy from above" look.
#define WATER_F0                0.02f
#define WATER_FRESNEL_POWER     5.0f

// clamp the fresnel so the surface never goes 100% mirror or 100% glass,
// otherwise it looks fake. taste, not physics.
#define WATER_FRESNEL_MIN       0.05f
#define WATER_FRESNEL_MAX       0.85f

// wave field. we sum a handful of gerstner waves on the cpu for the normal
// and let the gpu do the fine ripple. more than this and it turns to soup.
#define WATER_MAX_WAVES         6
#define WATER_DEFAULT_WAVES     4

// global wave amplitude/scale knobs, multiplied into every wave
#define WATER_WAVE_AMP_SCALE    1.0f
#define WATER_WAVE_TIME_SCALE   1.0f

// how steep gerstner waves are allowed to get before they pinch and self
// intersect. 0..1, keep it under ~0.8 or the crests fold over.
#define WATER_WAVE_STEEPNESS    0.6f

// depth-based tinting. shallow water is clear-ish, deep water goes murky.
// these are linear-space rgb, the shader handles the rest.
#define WATER_TINT_SHALLOW_R    0.25f
#define WATER_TINT_SHALLOW_G    0.55f
#define WATER_TINT_SHALLOW_B    0.60f
#define WATER_TINT_DEEP_R       0.02f
#define WATER_TINT_DEEP_G       0.10f
#define WATER_TINT_DEEP_B       0.18f

// how many blocks of depth it takes to reach full deep tint
#define WATER_TINT_DEPTH        12.0f

// distortion of the reflection/refraction sample, in uv units. tiny.
#define WATER_DISTORT_STRENGTH  0.03f

// caustics projector grid. cheap animated light pattern on the lakebed.
#define WATER_CAUSTIC_TILES     8
#define WATER_CAUSTIC_SPEED     0.35f

#endif
