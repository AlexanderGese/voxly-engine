#include "renderer.h"
#include "mesher.h"
#include "skybox.h"
#include "../util/log.h"
#include "../math/mat4.h"

#include <stdlib.h>

int renderer_init(renderer *r, const char *atlas_path) {
    r->prog_block  = gl_load_shader("shaders/block.vert", "shaders/block.frag");
    r->prog_skybox = gl_load_shader("shaders/skybox.vert","shaders/skybox.frag");
    r->prog_hud    = gl_load_shader("shaders/hud.vert",   "shaders/hud.frag");

    if (!r->prog_block || !r->prog_skybox || !r->prog_hud) {
        LOGE("renderer_init: shader load failed");
        return 0;
    }
    if (!texture_load(&r->atlas, atlas_path)) return 0;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.55f, 0.74f, 0.9f, 1.0f);

    r->chunk_meshes = NULL;
    r->meshes_cap = 0;
    return 1;
}

void renderer_shutdown(renderer *r) {
    gl_delete_shader(r->prog_block);
    gl_delete_shader(r->prog_skybox);
    gl_delete_shader(r->prog_hud);
    texture_destroy(&r->atlas);
    // chunk meshes now live on the chunk structs, not here
    free(r->chunk_meshes);
}

typedef struct {
    renderer *r;
    world    *w;
} draw_ctx;

static void visit_draw(chunk *c, void *u) {
    draw_ctx *d = u;
    if (c->dirty) mesher_build_chunk(d->w, c);
    if (c->vertex_count > 0 && c->vao) {
        glBindVertexArray(c->vao);
        glDrawArrays(GL_TRIANGLES, 0, c->vertex_count);
    }
}

void renderer_draw(renderer *r, world *w, const camera *cam) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 view = camera_view(cam);
    mat4 proj = camera_proj(cam);

    // skybox first
    skybox_draw(r->prog_skybox, cam);

    // blocks
    glUseProgram(r->prog_block);
    gl_set_uniform_mat4(r->prog_block, "u_view", mat4_data(&view));
    gl_set_uniform_mat4(r->prog_block, "u_proj", mat4_data(&proj));
    gl_set_uniform_int (r->prog_block, "u_atlas", 0);
    texture_bind(&r->atlas, 0);

    draw_ctx dc = { r, w };
    world_visit(w, visit_draw, &dc);
}
