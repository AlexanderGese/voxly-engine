#ifndef RENDER_DECALS_PASS_H
#define RENDER_DECALS_PASS_H

#include "decals_types.h"
#include "decals_pool.h"
#include "decals_cull.h"
#include "decals_atlas.h"
#include "decals_mesh.h"
#include "decals_program.h"
#include "../../math/mat4.h"

// the draw pass. given a culled+sorted visible list, builds the instance
// payloads, sets up blending into the live g-buffer, and stamps the projector
// cubes in BATCH_MAX-sized chunks. this is where the cpu-side state lives that
// the renderer needs to bind the g-buffer textures (depth + normal) for us.

typedef struct {
    decals_mesh    mesh;
    decals_program prog;

    // matrices fed in per frame. inv_view_proj is what the fs uses to turn
    // depth back into a world position.
    mat4 view_proj;
    mat4 inv_view_proj;
    int  screen_w, screen_h;

    int  drawn_last;   // instances actually drawn last frame, for stats
} decals_pass;

int  decals_pass_init(decals_pass *pass);
void decals_pass_shutdown(decals_pass *pass);

// hand the pass this frame's camera. caches the inverse for reconstruction.
void decals_pass_set_camera(decals_pass *pass, mat4 view_proj, mat4 inv_view_proj);
void decals_pass_set_screen(decals_pass *pass, int w, int h);

// the actual draw. `depth_tex` and `gnormal_tex` are the live g-buffer
// attachments to sample from; the pass writes into whatever fbo is currently
// bound (the renderer keeps the g-buffer bound, with blending). returns the
// instance count drawn.
int  decals_pass_draw(decals_pass *pass, const decals_pool *pool,
                      const decals_cull_result *vis, const decals_atlas *atlas,
                      glid depth_tex, glid gnormal_tex);

#endif
