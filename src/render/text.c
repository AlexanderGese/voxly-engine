#include "text.h"
#include "../util/log.h"

#include <string.h>

#define MAX_TEXT_VERTS (4096 * 6 * 4)

int text_init(text_renderer *t) {
    if (!font_init(&t->f)) return 0;
    t->prog = gl_load_shader("shaders/text.vert", "shaders/text.frag");
    if (!t->prog) {
        LOGE("text shader load failed");
        return 0;
    }
    glGenVertexArrays(1, &t->vao);
    glGenBuffers(1, &t->vbo);
    glBindVertexArray(t->vao);
    glBindBuffer(GL_ARRAY_BUFFER, t->vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_TEXT_VERTS * sizeof(float),
                 NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
    return 1;
}

void text_destroy(text_renderer *t) {
    font_destroy(&t->f);
    if (t->vao) glDeleteVertexArrays(1, &t->vao);
    if (t->vbo) glDeleteBuffers(1, &t->vbo);
    gl_delete_shader(t->prog);
}

void text_draw(text_renderer *t, const char *s, int x, int y,
               float r, float g, float b, int sw, int sh) {
    static float verts[MAX_TEXT_VERTS];
    int written = font_build_text(&t->f, s, x, y, verts, MAX_TEXT_VERTS);
    if (written == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, t->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, written * sizeof(float), verts);

    glUseProgram(t->prog);
    gl_set_uniform_float(t->prog, "u_sw", (float)sw);
    gl_set_uniform_float(t->prog, "u_sh", (float)sh);
    gl_set_uniform_vec3 (t->prog, "u_color", r, g, b);
    gl_set_uniform_int  (t->prog, "u_tex", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t->f.tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(t->vao);
    glDrawArrays(GL_TRIANGLES, 0, written / 4);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
