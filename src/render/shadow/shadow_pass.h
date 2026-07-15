#ifndef RENDER_SHADOW_PASS_H
#define RENDER_SHADOW_PASS_H

#include "shadow_types.h"
#include "shadow_map.h"

// the depth pass. for each cascade we bind its layer, set the light view_proj
// uniform, and let the caller draw whatever casts shadows. the caller hands us
// a callback so the shadow module doesnt need to know about chunks or mobs.

// called once per cascade. user gets the light view_proj for that cascade and
// its frustum (for culling casters) plus their own opaque ctx pointer.
typedef void (*shadow_caster_fn)(int cascade, mat4 light_view_proj, void *ctx);

typedef struct {
    glid prog_depth;     // the depth-only shader (position in, no color out)
    glid u_view_proj;    // cached uniform location
    int  cull_front;     // render back faces into the map to fight peter pan
} shadow_pass;

int  shadow_pass_init(shadow_pass *sp);
void shadow_pass_shutdown(shadow_pass *sp);

// run every cascade's depth pass. restores gl state (viewport is *not*
// restored — the caller owns that since it knows the real backbuffer size).
void shadow_pass_render(shadow_pass *sp, shadow_map *sm,
                        const shadow_csm *csm,
                        shadow_caster_fn draw, void *ctx);

#endif
