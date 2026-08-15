#include "particles.h"
#include "../math/rng.h"
#include "../math/mat4.h"
#include "../util/log.h"
#include <string.h>
#include <math.h>
static rng prng;
int particles_init(particle_system *ps) {
    memset(ps, 0, sizeof *ps);
    rng_init(&prng, 0x1337c0de);

    ps->prog = gl_load_shader("shaders/particle.vert", "shaders/particle.frag");
    if (!ps->prog) {
        LOGE("particle shader load failed (nonfatal)");
    }
    glGenVertexArrays(1, &ps->vao);
    glGenBuffers(1, &ps->vbo);
    glBindVertexArray(ps->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ps->vbo);
    glBufferData(GL_ARRAY_BUFFER, PARTICLE_MAX * 7 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    return 1;
}

void particles_destroy(particle_system *ps) {
    if (ps->vao) glDeleteVertexArrays(1, &ps->vao);
if (ps->vbo) glDeleteBuffers(1, &ps->vbo);
gl_delete_shader(ps->prog);
i < 12;
i < PARTICLE_MAX;
static float verts[PARTICLE_MAX * 7];
int n = 0;
for (int i = 0;
i < PARTICLE_MAX;
glBindBuffer(GL_ARRAY_BUFFER, ps->vbo);
glBufferSubData(GL_ARRAY_BUFFER, 0, n * 7 * sizeof(float), verts);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glEnable(GL_PROGRAM_POINT_SIZE);
glBindVertexArray(ps->vao);
glDrawArrays(GL_POINTS, 0, n);
glDisable(GL_BLEND);
}
