#ifndef RENDER_GBUFFER_H
#define RENDER_GBUFFER_H

#include "gbuffer_target.h"
#include "gbuffer_light.h"
#include "gbuffer_cull.h"
#include "../gl.h"
#include "../../math/mat4.h"

// top-level deferred pipeline object. owns the g-buffer target, the light
// accumulation buffer, all the pass shaders, and the fullscreen triangle we
// blit with. the renderer drives it: geometry -> accumulate -> shade.
//
// flow per frame:
// gbuffer_begin_geometry()   bind mrt, clear
// ... draw world into it with prog_geometry ...
// gbuffer_end_geometry()
// gbuffer_accumulate(lights)  splat each light into the accum buffer
// gbuffer_shade()             combine albedo*light + emissive -> screen

typedef enum {
    GBUFFER_VIEW_FINAL = 0,
    GBUFFER_VIEW_ALBEDO,
    GBUFFER_VIEW_NORMAL,
    GBUFFER_VIEW_MATERIAL,
    GBUFFER_VIEW_DEPTH,
    GBUFFER_VIEW_LIGHT,      // raw accumulation buffer
    GBUFFER_VIEW_COUNT
} gbuffer_view_mode;

typedef struct {
    gbuffer_target target;

    // light accumulation buffer (RGBA16F, additive). separate fbo so we can
    // keep the g-buffer intact while we splat lights into this.
    glid accum_fbo;
    glid accum_tex;

    glid prog_geometry;   // writes the g-buffer
    glid prog_accum;      // one fullscreen pass per light batch
    glid prog_shade;      // final combine + tonemap
    glid prog_debug;      // channel visualiser

    glid fs_vao;          // fullscreen triangle, no vbo (gl_VertexID trick)

    mat4 view, proj, inv_proj;
    int  w, h;

    gbuffer_view_mode view_mode;
    gbuffer_cull_result culled;   // last frame's culled set, for stats/debug
    float ambient[3];             // flat ambient term fed into shade
} gbuffer;

int  gbuffer_init(gbuffer *g, int w, int h);
void gbuffer_shutdown(gbuffer *g);
void gbuffer_resize(gbuffer *g, int w, int h);

// hand the pipeline this frame's camera matrices. caches inverse proj for
// the position reconstruction the accum/shade passes need.
void gbuffer_set_camera(gbuffer *g, mat4 view, mat4 proj);

void gbuffer_set_ambient(gbuffer *g, vec3 ambient);
void gbuffer_set_view_mode(gbuffer *g, gbuffer_view_mode m);

#endif
