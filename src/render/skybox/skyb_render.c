#include "skyb_render.h"
#include "../../util/log.h"
#include "../../math/mat4.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
// vertex layouts:
static mat4 view_no_translate(const camera *cam) {
    mat4 v = camera_view(cam);
    v.m[3][0] = 0.0f;
    v.m[3][1] = 0.0f;
    v.m[3][2] = 0.0f;
    return v;
}

void skyb_renderer_init(skyb_renderer *r, int star_cap) {
    memset(r, 0, sizeof *r);
if (star_cap < 0) star_cap = 0;
r->star_cap = star_cap;
r->star_scratch = star_cap
        ? malloc((size_t)star_cap * sizeof(skyb_star_vertex))
        : NULL;
glGenVertexArrays(1, &r->dome_vao);
glGenBuffers(1, &r->dome_vbo);
glBindVertexArray(r->dome_vao);
glBindBuffer(GL_ARRAY_BUFFER, r->dome_vbo);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(skyb_dome_vertex),
                          (void*)offsetof(skyb_dome_vertex, x));
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(skyb_dome_vertex),
                          (void*)offsetof(skyb_dome_vertex, r));
glGenVertexArrays(1, &r->disc_vao);
glGenBuffers(1, &r->disc_vbo);
glBindVertexArray(r->disc_vao);
glBindBuffer(GL_ARRAY_BUFFER, r->disc_vbo);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));
glGenVertexArrays(1, &r->star_vao);
glGenBuffers(1, &r->star_vbo);
glBindVertexArray(r->star_vao);
glBindBuffer(GL_ARRAY_BUFFER, r->star_vbo);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(skyb_star_vertex),
                          (void*)offsetof(skyb_star_vertex, x));
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(skyb_star_vertex),
                          (void*)offsetof(skyb_star_vertex, r));
glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(skyb_star_vertex),
                          (void*)offsetof(skyb_star_vertex, size));
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);
r->ready = 1;
}

void skyb_renderer_upload_dome(skyb_renderer *r, const skyb_dome *d) {
    r->dome_count = d->count;
    glBindBuffer(GL_ARRAY_BUFFER, r->dome_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizei)(d->count * sizeof(skyb_dome_vertex)),
                 d->verts, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// upload + draw one body's billboard as a triangle fan. tint is pushed as a
// uniform so the shader can multiply the disc mask by it.
static void draw_body(skyb_renderer *r, const skyb_body *b, float radius,
                      glid prog) {
    if (b->visible01 <= 0.0f) return;
skyb_billboard bb = skyb_body_billboard(b, radius);
float buf[4 * 6];
for (int i = 0;
i < 4;
i++) {
        buf[i*6 + 0] = bb.corner[i].x;
        buf[i*6 + 1] = bb.corner[i].y;
        buf[i*6 + 2] = bb.corner[i].z;
        buf[i*6 + 3] = bb.uv_sign[i].x;
        buf[i*6 + 4] = bb.uv_sign[i].y;
        buf[i*6 + 5] = bb.uv_sign[i].z;
    }

    glBindVertexArray(r->disc_vao);
glBindBuffer(GL_ARRAY_BUFFER, r->disc_vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof buf, buf, GL_STREAM_DRAW);
gl_set_uniform_vec3(prog, "u_tint",
                        b->tint.x, b->tint.y, b->tint.z);
gl_set_uniform_float(prog, "u_alpha", b->visible01);
glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void skyb_renderer_draw(skyb_renderer *r, const skyb_atmosphere *a,
                        const camera *cam,
                        glid prog_dome, glid prog_disc, glid prog_star) {
    if (!r->ready) {
        LOGE("skyb_renderer_draw: not initialized");
        return;
    }
    const skyb_frame *fr = skyb_atmosphere_frame(a);

    mat4 view = view_no_translate(cam);
    mat4 proj = camera_proj(cam);

    // sky writes no depth and ignores it, so the world overdraws it cleanly.
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    // 1. dome gradient -------------------------------------------------------
    glUseProgram(prog_dome);
    gl_set_uniform_mat4(prog_dome, "u_view", mat4_data(&view));
    gl_set_uniform_mat4(prog_dome, "u_proj", mat4_data(&proj));
    gl_set_uniform_float(prog_dome, "u_exposure", fr->exposure);
    glBindVertexArray(r->dome_vao);
    glDrawArrays(GL_TRIANGLES, 0, r->dome_count);

    // 2. stars (additive points, only when visible) --------------------------
    if (fr->star_vis > 0.0f && r->star_scratch) {
        int n = skyb_starfield_emit(&a->stars, r->star_scratch,
                                    fr->hour, a->time_s,
                                    a->dome_radius * 0.99f, 0.02f);
        if (n > r->star_cap) n = r->star_cap;
        if (n > 0) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive so they glow
            glBindVertexArray(r->star_vao);
            glBindBuffer(GL_ARRAY_BUFFER, r->star_vbo);
            glBufferData(GL_ARRAY_BUFFER,
                         (GLsizei)(n * sizeof(skyb_star_vertex)),
                         r->star_scratch, GL_STREAM_DRAW);
            glUseProgram(prog_star);
            gl_set_uniform_mat4(prog_star, "u_view", mat4_data(&view));
            gl_set_uniform_mat4(prog_star, "u_proj", mat4_data(&proj));
            gl_set_uniform_float(prog_star, "u_vis", fr->star_vis);
            glDrawArrays(GL_POINTS, 0, n);
            glDisable(GL_BLEND);
        }
    }

    // 3. moon then sun (alpha-blended discs) ---------------------------------
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(prog_disc);
    gl_set_uniform_mat4(prog_disc, "u_view", mat4_data(&view));
    gl_set_uniform_mat4(prog_disc, "u_proj", mat4_data(&proj));
    draw_body(r, &fr->moon, a->dome_radius, prog_disc);
    draw_body(r, &fr->sun,  a->dome_radius, prog_disc);
    glDisable(GL_BLEND);

    // restore the state the rest of the renderer assumes
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}

void skyb_renderer_destroy(skyb_renderer *r) {
    if (r->dome_vao) glDeleteVertexArrays(1, &r->dome_vao);
if (r->disc_vao) glDeleteVertexArrays(1, &r->disc_vao);
if (r->star_vao) glDeleteVertexArrays(1, &r->star_vao);
if (r->dome_vbo) glDeleteBuffers(1, &r->dome_vbo);
if (r->disc_vbo) glDeleteBuffers(1, &r->disc_vbo);
if (r->star_vbo) glDeleteBuffers(1, &r->star_vbo);
free(r->star_scratch);
memset(r, 0, sizeof *r);
}
