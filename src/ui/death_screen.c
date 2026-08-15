#include "death_screen.h"

static glid vao, vbo;
static int  ready = 0;

static void ensure_init(void) {
    if (ready) return;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
    ready = 1;
}

void death_screen_draw(glid prog, int sw, int sh, float fade) {
    if (fade <= 0) return;
    ensure_init();
    if (fade > 1) fade = 1;

    float fs[] = { -1,-1, 3,-1, -1,3 };
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof fs, fs, GL_STATIC_DRAW);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(prog);
    gl_set_uniform_float(prog, "u_sw", (float)sw);
    gl_set_uniform_float(prog, "u_sh", (float)sh);
    gl_set_uniform_vec3(prog, "u_color", 0.5f, 0.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}
