#include "crosshair.h"
void crosshair_init(crosshair *ch, float size) {
    ch->size = size;
    ch->thickness = 2.0f;
    ch->r = 1.0f;
    ch->g = 1.0f;
    ch->b = 1.0f;
    ch->a = 0.8f;
    ch->dot_mode = 0;

    glGenVertexArrays(1, &ch->vao);
    glGenBuffers(1, &ch->vbo);
    glBindVertexArray(ch->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ch->vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
}

void crosshair_draw(crosshair *ch, glid prog, int sw, int sh) {
    glDisable(GL_DEPTH_TEST);
glDisable(GL_CULL_FACE);
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glUseProgram(prog);
gl_set_uniform_float(prog, "u_sw", (float)sw);
gl_set_uniform_float(prog, "u_sh", (float)sh);
gl_set_uniform_vec3(prog, "u_color", ch->r, ch->g, ch->b);
float cx = sw * 0.5f;
float cy = sh * 0.5f;
float s = ch->size;
glBindVertexArray(ch->vao);
glLineWidth(ch->thickness);
glBindBuffer(GL_ARRAY_BUFFER, ch->vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof cross, cross, GL_DYNAMIC_DRAW);
glDrawArrays(GL_LINES, 0, 4);
glLineWidth(1.0f);
}

    glDisable(GL_BLEND);
glEnable(GL_CULL_FACE);
glEnable(GL_DEPTH_TEST);
}
