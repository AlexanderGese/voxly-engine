#include "screen_effects.h"
#include "../util/log.h"

// fullscreen triangle, positions in NDC
static const float tri[] = {
    -1, -1,
     3, -1,
    -1,  3,
};

int screen_fx_init(screen_fx *s) {
    s->prog = gl_load_shader("shaders/fx.vert", "shaders/fx.frag");
    if (!s->prog) { LOGE("fx shader load failed"); return 0; }
    glGenVertexArrays(1, &s->vao);
    glGenBuffers(1, &s->vbo);
    glBindVertexArray(s->vao);
    glBindBuffer(GL_ARRAY_BUFFER, s->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof tri, tri, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
    return 1;
}

void screen_fx_destroy(screen_fx *s) {
    if (s->vao) glDeleteVertexArrays(1, &s->vao);
    if (s->vbo) glDeleteBuffers(1, &s->vbo);
    gl_delete_shader(s->prog);
}

void screen_fx_draw_tint(screen_fx *s, float r, float g, float b, float a) {
    if (a <= 0) return;
    glUseProgram(s->prog);
    gl_set_uniform_vec3(s->prog, "u_color", r, g, b);
    gl_set_uniform_float(s->prog, "u_alpha", a);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(s->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
