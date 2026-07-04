#include "debugdraw_gl.h"
#include "../../util/log.h"

#include <stddef.h>

int ddgl_init(ddgl *g) {
    g->prog = gl_load_shader("shaders/debugdraw.vert", "shaders/debugdraw.frag");
    if (!g->prog) {
        LOGE("debugdraw shader load failed");
        return 0;
    }

    glGenVertexArrays(1, &g->vao);
    glGenBuffers(1, &g->vbo);
    glBindVertexArray(g->vao);
    glBindBuffer(GL_ARRAY_BUFFER, g->vbo);

    // start with room for a few thousand verts, grows later if we overflow
    g->vbo_bytes = 4096 * DDVERT_STRIDE;
    glBufferData(GL_ARRAY_BUFFER, g->vbo_bytes, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, DDVERT_STRIDE,
                          (void*)(size_t)DDVERT_POS_OFFSET);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, DDVERT_STRIDE,
                          (void*)(size_t)DDVERT_COL_OFFSET);

    glBindVertexArray(0);
    return 1;
}

void ddgl_destroy(ddgl *g) {
    if (g->vao) glDeleteVertexArrays(1, &g->vao);
    if (g->vbo) glDeleteBuffers(1, &g->vbo);
    gl_delete_shader(g->prog);
    g->vao = g->vbo = 0;
    g->prog = 0;
}

void ddgl_upload(ddgl *g, const ddvert *verts, int count) {
    if (count <= 0) return;
    int need = count * DDVERT_STRIDE;

    glBindVertexArray(g->vao);
    glBindBuffer(GL_ARRAY_BUFFER, g->vbo);

    if (need > g->vbo_bytes) {
        // grow to next pow2-ish so we dont realloc every single frame
        int nb = g->vbo_bytes;
        while (nb < need) nb *= 2;
        glBufferData(GL_ARRAY_BUFFER, nb, NULL, GL_DYNAMIC_DRAW);
        g->vbo_bytes = nb;
        LOGD("debugdraw vbo grown to %d bytes", nb);
    }

    // orphan + refill. the NULL bufferdata above (when grown) already
    // orphans; when not grown we just overwrite from 0.
    glBufferSubData(GL_ARRAY_BUFFER, 0, need, verts);
}

void ddgl_begin(ddgl *g, const float *viewproj) {
    glUseProgram(g->prog);
    gl_set_uniform_mat4(g->prog, "u_viewproj", viewproj);
    glBindVertexArray(g->vao);

    // debug lines look better blended, and points get rounded in the frag
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ddgl_draw_range(ddgl *g, unsigned mode, int first, int count,
                     int depth_test, float line_width, float point_size) {
    if (count <= 0) return;

    if (depth_test) glEnable(GL_DEPTH_TEST);
    else            glDisable(GL_DEPTH_TEST);

    if (mode == GL_LINES || mode == GL_LINE_STRIP || mode == GL_LINE_LOOP)
        glLineWidth(line_width);

    // point size travels via uniform so the shader can set gl_PointSize;
    // GL_PROGRAM_POINT_SIZE has to be on for that to take effect.
    if (mode == GL_POINTS) {
        glEnable(GL_PROGRAM_POINT_SIZE);
        gl_set_uniform_float(g->prog, "u_point_size", point_size);
    }

    glDrawArrays(mode, first, count);

    if (mode == GL_POINTS)
        glDisable(GL_PROGRAM_POINT_SIZE);
}

void ddgl_end(ddgl *g) {
    (void)g;
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLineWidth(1.0f);
}
