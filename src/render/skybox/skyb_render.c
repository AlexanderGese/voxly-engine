#include "skyb_render.h"
#include "../../util/log.h"
#include "../../math/mat4.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
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
skyb_billboard bb = skyb_body_billboard(b, radius);
float buf[4 * 6];
for (int i = 0;
i < 4;
glBindBuffer(GL_ARRAY_BUFFER, r->disc_vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof buf, buf, GL_STREAM_DRAW);
gl_set_uniform_vec3(prog, "u_tint",
                        b->tint.x, b->tint.y, b->tint.z);
gl_set_uniform_float(prog, "u_alpha", b->visible01);
glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
if (r->disc_vao) glDeleteVertexArrays(1, &r->disc_vao);
if (r->star_vao) glDeleteVertexArrays(1, &r->star_vao);
if (r->dome_vbo) glDeleteBuffers(1, &r->dome_vbo);
if (r->disc_vbo) glDeleteBuffers(1, &r->disc_vbo);
if (r->star_vbo) glDeleteBuffers(1, &r->star_vbo);
free(r->star_scratch);
memset(r, 0, sizeof *r);
}
