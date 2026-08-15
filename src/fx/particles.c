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
}

static int alloc_slot(particle_system *ps) {
    for (int i = 0; i < PARTICLE_MAX; i++) {
        if (!ps->list[i].alive) return i;
    }
    return -1;
}

void particles_spawn_break(particle_system *ps, vec3 center) {
    for (int i = 0; i < 12; i++) {
        int k = alloc_slot(ps);
        if (k < 0) break;
        particle *p = &ps->list[k];
        p->pos = vec3_add(center, (vec3){
            rng_frange(&prng, -0.2f, 0.2f),
            rng_frange(&prng, -0.2f, 0.2f),
            rng_frange(&prng, -0.2f, 0.2f),
        });
        p->vel = (vec3){
            rng_frange(&prng, -1, 1),
            rng_frange(&prng,  2, 4),
            rng_frange(&prng, -1, 1),
        };
        p->age = 0;
        p->life = rng_frange(&prng, 0.5f, 1.0f);
        p->size = rng_frange(&prng, 0.06f, 0.1f);
        p->r = 0.6f; p->g = 0.4f; p->b = 0.2f;
        p->alive = 1;
        ps->count++;
    }
}

void particles_update(particle_system *ps, float dt) {
    for (int i = 0; i < PARTICLE_MAX; i++) {
        particle *p = &ps->list[i];
        if (!p->alive) continue;
        p->age += dt;
        if (p->age >= p->life) {
            p->alive = 0;
            ps->count--;
            continue;
        }
        p->vel.y += -9.8f * dt;
        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;
        p->pos.z += p->vel.z * dt;
    }
}

void particles_draw(particle_system *ps, const camera *cam) {
    if (ps->count == 0) return;

    static float verts[PARTICLE_MAX * 7];
    int n = 0;
    for (int i = 0; i < PARTICLE_MAX; i++) {
        particle *p = &ps->list[i];
        if (!p->alive) continue;
        float alpha = 1.0f - (p->age / p->life);
        verts[n * 7 + 0] = p->pos.x;
        verts[n * 7 + 1] = p->pos.y;
        verts[n * 7 + 2] = p->pos.z;
        verts[n * 7 + 3] = p->r;
        verts[n * 7 + 4] = p->g;
        verts[n * 7 + 5] = p->b;
        verts[n * 7 + 6] = alpha;
        n++;
    }
    if (n == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, ps->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, n * 7 * sizeof(float), verts);

    if (ps->prog) {
        glUseProgram(ps->prog);
        mat4 view = camera_view(cam);
        mat4 proj = camera_proj(cam);
        gl_set_uniform_mat4(ps->prog, "u_view", mat4_data(&view));
        gl_set_uniform_mat4(ps->prog, "u_proj", mat4_data(&proj));
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glBindVertexArray(ps->vao);
    glDrawArrays(GL_POINTS, 0, n);
    glDisable(GL_BLEND);
}
