#ifndef RENDER_SKYBOX_RENDER_H
#define RENDER_SKYBOX_RENDER_H

// the gl glue. owns the vaos/vbos for the dome, the sun/moon discs and the
// star points, and draws them in the right order with depth writes off so the
// world draws on top. expects the matching shaders to be loaded by the caller
// (renderer owns the programs, same as the rest of the engine).
//
// draw order each frame: dome gradient -> stars -> moon -> sun. the sky is
// rendered centered on the camera with translation stripped from the view so
// it stays "infinitely far".

#include "../gl.h"
#include "../camera.h"
#include "skyb_atmosphere.h"

typedef struct {
    glid dome_vao, dome_vbo;
    glid disc_vao, disc_vbo;   // reused for sun + moon, re-uploaded per draw
    glid star_vao, star_vbo;

    int  dome_count;           // verts uploaded for the dome
    int  star_cap;             // capacity of the star scratch buffer

    skyb_star_vertex *star_scratch; // cpu scratch reused each frame
    int  ready;
} skyb_renderer;

// create gl objects + scratch. star_cap caps how many star points we emit.
void skyb_renderer_init(skyb_renderer *r, int star_cap);

// upload the (re-shaded) dome mesh. call after skyb_atmosphere_bake whenever
// the dome vertices changed (i.e. every frame, since we reshade every frame).
void skyb_renderer_upload_dome(skyb_renderer *r, const skyb_dome *d);

// draw the whole sky. `prog_*` are the shader programs to use for each pass.
// they can be the same program if it branches on a uniform; we set u_kind.
void skyb_renderer_draw(skyb_renderer *r, const skyb_atmosphere *a,
                        const camera *cam,
                        glid prog_dome, glid prog_disc, glid prog_star);

void skyb_renderer_destroy(skyb_renderer *r);

#endif
