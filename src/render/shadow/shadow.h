#ifndef RENDER_SHADOW_H
#define RENDER_SHADOW_H

#include "shadow_types.h"
#include "shadow_map.h"
#include "shadow_pass.h"
#include "shadow_pcf.h"
#include "shadow_cull.h"
#include "shadow_stabilize.h"
#include "../camera.h"

// top level cascaded shadow map facade. this is the only header the renderer
// needs to include. everything underneath (split/frustum/bounds/matrix/map/
// pass/pcf) is wired together here.
//
// typical frame:
// shadow_update(&sh, &cam, sun_dir);          // recompute cascades
// shadow_render(&sh, draw_casters, ctx);      // depth passes
// ... bind main shader ...
// shadow_bind(&sh, main_prog, /*tex unit*/ 4);// samplers + matrices
// ... draw scene lit ...

typedef struct {
    shadow_csm        csm;
    shadow_map        map;
    shadow_pass       pass;
    shadow_pcf_kernel pcf;
    shadow_cull       cull;
    shadow_stabilizer stab;
    int               ready;
} shadow_system;

int  shadow_init(shadow_system *sh);
void shadow_shutdown(shadow_system *sh);

// recompute splits + per-cascade matrices for this camera and sun direction.
// dt drives the light-direction slew that keeps edges from crawling.
void shadow_update(shadow_system *sh, const camera *cam, vec3 sun_dir, float dt);

// run all the cascade depth passes, calling draw() once per cascade.
void shadow_render(shadow_system *sh, shadow_caster_fn draw, void *ctx);

// bind the depth array + upload matrices/splits/pcf to the lighting shader.
void shadow_bind(shadow_system *sh, glid lighting_prog, int tex_unit);

// toggle. when off, shadow_render is a no-op and the shader should treat
// everything as lit (the bound depth map still reads 1.0 at the border).
void shadow_set_enabled(shadow_system *sh, int on);

// caster culling: does this world-space box need drawing into cascade i?
// the depth-pass callback should gate its per-chunk draws on this.
int  shadow_caster_visible(const shadow_system *sh, int cascade, aabb box);

#endif
