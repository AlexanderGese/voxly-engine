#include "crack.h"
#include "../math/mat4.h"
#include "../util/log.h"

// 6 faces, 2 tris each, 3 verts -> 36 tris
static const float cube[] = {
    0,0,0, 1,0,0, 1,1,0,   0,0,0, 1,1,0, 0,1,0,
    0,0,1, 1,0,1, 1,1,1,   0,0,1, 1,1,1, 0,1,1,
    0,0,0, 0,1,0, 0,1,1,   0,0,0, 0,1,1, 0,0,1,
    1,0,0, 1,1,0, 1,1,1,   1,0,0, 1,1,1, 1,0,1,
    0,1,0, 1,1,0, 1,1,1,   0,1,0, 1,1,1, 0,1,1,
    0,0,0, 1,0,0, 1,0,1,   0,0,0, 1,0,1, 0,0,1,
};

int crack_init(crack_overlay *co) {
    co->prog = gl_load_shader("shaders/crack.vert", "shaders/crack.frag");
    if (!co->prog) {
        LOGE("crack shader load failed");
        return 0;
    }
    glGenVertexArrays(1, &co->vao);
    glGenBuffers(1, &co->vbo);
    glBindVertexArray(co->vao);
    glBindBuffer(GL_ARRAY_BUFFER, co->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof cube, cube, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glBindVertexArray(0);
    return 1;
}

void crack_destroy(crack_overlay *co) {
    if (co->vao) glDeleteVertexArrays(1, &co->vao);
    if (co->vbo) glDeleteBuffers(1, &co->vbo);
    gl_delete_shader(co->prog);
}

void crack_draw(crack_overlay *co, int bx, int by, int bz, int phase,
                const camera *cam) {
    if (phase <= 0) return;
    if (phase > 10) phase = 10;

    mat4 view = camera_view(cam);
    mat4 proj = camera_proj(cam);
    mat4 t    = mat4_translate((vec3){ (float)bx - 0.001f, (float)by - 0.001f, (float)bz - 0.001f });
    mat4 s    = mat4_scale((vec3){1.002f, 1.002f, 1.002f});
    mat4 model= mat4_mul(t, s);

    glUseProgram(co->prog);
    gl_set_uniform_mat4(co->prog, "u_view",  mat4_data(&view));
    gl_set_uniform_mat4(co->prog, "u_proj",  mat4_data(&proj));
    gl_set_uniform_mat4(co->prog, "u_model", mat4_data(&model));
    gl_set_uniform_float(co->prog, "u_phase", (float)phase / 10.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(co->vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDisable(GL_BLEND);
}
