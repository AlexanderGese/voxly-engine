#include "system.h"
#include "sort.h"

#include "../../util/log.h"

#include <stdlib.h>
#include <string.h>

void particles_system_init(particles_system *s, int capacity, int needs_sort) {
    memset(s, 0, sizeof *s);
    particles_pool_init(&s->pool, capacity);

    s->emitter_count = 0;
    s->needs_sort    = needs_sort;
    particles_sim_env_default(&s->env);

    // each live particle expands to 6 verts. size the scratch to the worst
    // case so the billboard stage never has to bail mid-frame.
    s->scratch_cap = s->pool.capacity * PARTICLES_VERTS_PER;
    s->scratch     = malloc((size_t)s->scratch_cap * sizeof(particles_vertex));
    if (!s->scratch) {
        LOGE("particles_system_init: scratch OOM (%d verts)", s->scratch_cap);
        s->scratch_cap = 0;
    }

    particles_gpu_init(&s->gpu, s->scratch_cap ? s->scratch_cap : 64);
    s->has_bounds = 0;
}

void particles_system_destroy(particles_system *s) {
    particles_gpu_destroy(&s->gpu);
    particles_pool_free(&s->pool);
    free(s->scratch);
    memset(s, 0, sizeof *s);
}

int particles_system_add_emitter(particles_system *s, const particles_emitter *e) {
    if (s->emitter_count >= PARTICLES_MAX_EMITTERS) return -1;
    int id = s->emitter_count++;
    s->emitters[id] = *e;
    s->emitters[id].id = (uint16_t)id;   // make sure the stamp matches the slot
    return id;
}

particles_emitter *particles_system_emitter(particles_system *s, int id) {
    if (id < 0 || id >= s->emitter_count) return NULL;
    return &s->emitters[id];
}

int particles_system_burst(particles_system *s, int emitter_id, int count) {
    particles_emitter *e = particles_system_emitter(s, emitter_id);
    if (!e) return 0;
    int n = particles_emitter_burst(e, &s->pool, count);
    s->last_spawned += n;
    return n;
}

void particles_system_update(particles_system *s, float dt) {
    if (dt < 0.0f) dt = 0.0f;
    s->last_spawned = 0;

    // continuous emission from every enabled emitter
    for (int i = 0; i < s->emitter_count; i++) {
        s->last_spawned += particles_emitter_emit(&s->emitters[i], &s->pool, dt);
    }

    // integrate + kill expired
    particles_simulate(&s->pool, s->emitters, s->emitter_count, &s->env, dt);

    // refresh the dense live list and the cached bounds
    s->last_live = particles_pool_collect(&s->pool);
    s->has_bounds = particles_compute_bounds(&s->pool, &s->bounds);
}

void particles_system_render(particles_system *s,
                             vec3 cam_right, vec3 cam_up,
                             vec3 cam_fwd, vec3 eye) {
    if (s->pool.live_count == 0 || s->scratch_cap == 0) return;

    // alpha systems need far-to-near ordering; additive doesn't, so we save
    // the sort entirely there.
    if (s->needs_sort) {
        particles_sort_back_to_front(&s->pool, eye);
    }

    particles_billboard_cam bc;
    particles_billboard_cam_make(&bc, cam_right, cam_up, cam_fwd, eye);

    s->scratch_used = particles_billboard_build(
        s->scratch, s->scratch_cap,
        &s->pool, s->emitters, s->emitter_count, &bc);

    particles_gpu_upload(&s->gpu, s->scratch, s->scratch_used);

    // blend state. particles are never depth-written (they'd occlude each
    // other wrong) but they do depth-test against the world.
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    // the system is single-blend-mode at the gpu level; we pick it from the
    // first emitter since mixing additive + alpha in one buffer would need a
    // per-batch split anyway (a TODO for the day we actually need it).
    particles_blend_mode mode =
        s->emitter_count > 0 ? s->emitters[0].blend : PARTICLES_BLEND_ALPHA;
    if (mode == PARTICLES_BLEND_ADD) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    particles_gpu_draw(&s->gpu);

    // restore the defaults the rest of the renderer expects
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
