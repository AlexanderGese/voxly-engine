#ifndef RENDER_SHADOW_DEBUG_H
#define RENDER_SHADOW_DEBUG_H

#include "shadow_types.h"

// dev-only helpers for figuring out why the shadows look wrong this time.
// none of this runs in a normal frame — its all behind F-key toggles in the
// renderer. kept out of shadow.c so the hot path stays lean.

// a distinct rgb tint per cascade so you can SEE the split boundaries when you
// colour fragments by their selected cascade in the shader.
vec3 shadow_debug_cascade_color(int cascade);

// dump the split distances + each cascade's ortho box extents to the log.
// call it once when you tweak a constant, not every frame.
void shadow_debug_dump(const shadow_csm *csm);

// the 12 edges of a cascade's frustum slice, as line-list vertex pairs, in
// world space. out must hold 24 vec3. returns the count written (24).
int  shadow_debug_slice_lines(const shadow_csm *csm, int cascade,
                              mat4 cam_view, float fov_rad, float aspect,
                              vec3 *out);

// approximate texel density (texels per world unit) for a cascade. lower means
// blockier shadows — useful for sanity checking the split tuning.
float shadow_debug_texel_density(const shadow_csm *csm, int cascade);

#endif
