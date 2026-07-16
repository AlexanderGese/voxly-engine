#ifndef RENDER_SKYBOX_RENDER_H
#define RENDER_SKYBOX_RENDER_H
// the gl glue. owns the vaos/vbos for the dome, the sun/moon discs and the
// star points, and draws them in the right order with depth writes off so the
// world draws on top. expects the matching shaders to be loaded by the caller
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
void skyb_renderer_init(skyb_renderer *r, int star_cap);
void skyb_renderer_upload_dome(skyb_renderer *r, const skyb_dome *d);
void skyb_renderer_draw(skyb_renderer *r, const skyb_atmosphere *a,
                        const camera *cam,
                        glid prog_dome, glid prog_disc, glid prog_star);
void skyb_renderer_destroy(skyb_renderer *r);
#endif
