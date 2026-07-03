#include "skybox.h"
#include "../math/mat4.h"

// a fullscreen triangle is enough
static const float fs_tri[] = {
    -1.f, -1.f,
     3.f, -1.f,
    -1.f,  3.f,
};

static glid g_vao, g_vbo;
static int  g_ready = 0;

void skybox_init(void) {
    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof fs_tri, fs_tri, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
    g_ready = 1;
}

void skybox_draw(glid prog, const camera *cam) {
    if (!g_ready) skybox_init();

    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glUseProgram(prog);
    gl_set_uniform_float(prog, "u_pitch", cam->pitch);
    glBindVertexArray(g_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}
