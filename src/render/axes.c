#include "axes.h"
#include "../math/mat4.h"
#include "../util/log.h"

static const float verts[] = {
    0,0,0, 1,0,0, 1,0,0, 1,0,0,
    0,0,0, 0,1,0, 0,1,0, 0,1,0,
    0,0,0, 0,0,1, 0,0,1, 0,0,1,
};

int axes_init(axes_gizmo *a) {
    a->prog = gl_load_shader("shaders/axes.vert", "shaders/axes.frag");
    if (!a->prog) { LOGE("axes shader failed"); return 0; }
    glGenVertexArrays(1, &a->vao);
    glGenBuffers(1, &a->vbo);
    glBindVertexArray(a->vao);
    glBindBuffer(GL_ARRAY_BUFFER, a->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    return 1;
}

void axes_destroy(axes_gizmo *a) {
    if (a->vao) glDeleteVertexArrays(1, &a->vao);
    if (a->vbo) glDeleteBuffers(1, &a->vbo);
    gl_delete_shader(a->prog);
}

void axes_draw(axes_gizmo *a, const camera *cam, float scale) {
    mat4 view = camera_view(cam);
    mat4 proj = camera_proj(cam);
    mat4 sc   = mat4_scale((vec3){scale, scale, scale});
    glUseProgram(a->prog);
    gl_set_uniform_mat4(a->prog, "u_view",  mat4_data(&view));
    gl_set_uniform_mat4(a->prog, "u_proj",  mat4_data(&proj));
    gl_set_uniform_mat4(a->prog, "u_model", mat4_data(&sc));
    glBindVertexArray(a->vao);
    glDrawArrays(GL_LINES, 0, 6);
}
