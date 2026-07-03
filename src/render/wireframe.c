#include "wireframe.h"
#include "../math/mat4.h"
#include "../util/log.h"

// 12 edges of a unit cube, origin (0,0,0) to (1,1,1)
static const float cube_edges[] = {
    0,0,0, 1,0,0,   1,0,0, 1,0,1,   1,0,1, 0,0,1,   0,0,1, 0,0,0,
    0,1,0, 1,1,0,   1,1,0, 1,1,1,   1,1,1, 0,1,1,   0,1,1, 0,1,0,
    0,0,0, 0,1,0,   1,0,0, 1,1,0,   1,0,1, 1,1,1,   0,0,1, 0,1,1,
};

int wireframe_init(wireframe *wf) {
    wf->prog = gl_load_shader("shaders/wire.vert", "shaders/wire.frag");
    if (!wf->prog) {
        LOGE("wire shader load failed");
        return 0;
    }
    glGenVertexArrays(1, &wf->vao);
    glGenBuffers(1, &wf->vbo);
    glBindVertexArray(wf->vao);
    glBindBuffer(GL_ARRAY_BUFFER, wf->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof cube_edges, cube_edges, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glBindVertexArray(0);
    return 1;
}

void wireframe_destroy(wireframe *wf) {
    if (wf->vao) glDeleteVertexArrays(1, &wf->vao);
    if (wf->vbo) glDeleteBuffers(1, &wf->vbo);
    gl_delete_shader(wf->prog);
}

void wireframe_draw_aabb(wireframe *wf, aabb a, const camera *cam,
                         float r, float g, float b) {
    vec3 size = vec3_sub(a.max, a.min);
    mat4 t  = mat4_translate(a.min);
    mat4 sc = mat4_scale(size);
    mat4 model = mat4_mul(t, sc);
    mat4 view  = camera_view(cam);
    mat4 proj  = camera_proj(cam);

    glUseProgram(wf->prog);
    gl_set_uniform_mat4(wf->prog, "u_model", mat4_data(&model));
    gl_set_uniform_mat4(wf->prog, "u_view",  mat4_data(&view));
    gl_set_uniform_mat4(wf->prog, "u_proj",  mat4_data(&proj));
    gl_set_uniform_vec3(wf->prog, "u_color", r, g, b);

    glLineWidth(2.0f);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(wf->vao);
    glDrawArrays(GL_LINES, 0, 24);
    glEnable(GL_DEPTH_TEST);
}
