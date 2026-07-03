#include "water_render.h"
#include "../math/mat4.h"
#include "../util/log.h"

int water_renderer_init(water_renderer *wr) {
    wr->prog = gl_load_shader("shaders/water.vert", "shaders/water.frag");
    if (!wr->prog) {
        LOGW("water shader not found (nonfatal)");
        return 0;
    }
    return 1;
}

void water_renderer_destroy(water_renderer *wr) {
    gl_delete_shader(wr->prog);
}

void water_renderer_draw(water_renderer *wr, world *w, const camera *cam, float time) {
    if (!wr->prog) return;
    // water uses the same chunk VAOs as opaque blocks — we just need to
    // draw chunks with a different shader that applies transparency +
    // a time-based vertex offset for waves.

    mat4 view = camera_view(cam);
    mat4 proj = camera_proj(cam);

    glUseProgram(wr->prog);
    gl_set_uniform_mat4(wr->prog, "u_view", mat4_data(&view));
    gl_set_uniform_mat4(wr->prog, "u_proj", mat4_data(&proj));
    gl_set_uniform_float(wr->prog, "u_time", time);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    // ideally we'd have a separate water mesh per chunk, but for now
    // this is a stub that sets up the shader state. the actual drawing
    // happens in the main chunk pass (water blocks are in the same mesh).
    // TODO: separate water mesh pass

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
