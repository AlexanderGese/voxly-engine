#ifndef RENDER_DECALS_GLSL_H
#define RENDER_DECALS_GLSL_H

// inline shader source for the deferred decal pass. kept as string constants
// (like shadow_glsl) so the decal subsystem doesnt depend on shader files
// shipping alongside the binary — the projector cube + reconstruction logic is
// small enough to live here and never drifts out of sync with the c side.

// vertex stage: draws a unit cube per instance, positioned by the instance's
// inverse-model (we pass model, the fs uses inv_model for the local transform).
extern const char *DECALS_GLSL_VERT;

// fragment stage: reconstruct world pos from the g-buffer depth, fold into the
// projector's local cube, clip, sample the atlas, blend albedo/normal/material.
extern const char *DECALS_GLSL_FRAG;

#endif
