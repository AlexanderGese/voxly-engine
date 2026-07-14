#include "shadow.h"
#include "shadow_split.h"
#include "shadow_matrix.h"
#include "shadow_glsl.h"
#include "../../util/log.h"

int shadow_init(shadow_system *sh) {
    sh->ready = 0;
    sh->csm.enabled = 1;
    sh->csm.count   = SHADOW_CASCADE_COUNT;

    // make sure the depth shaders exist on disk before the pass loads them.
    shadow_glsl_write("shaders");

    if (!shadow_map_create(&sh->map, SHADOW_MAP_SIZE)) {
        LOGE("shadow: depth map creation failed");
        return 0;
    }
    if (!shadow_pass_init(&sh->pass)) {
        LOGE("shadow: depth pass init failed");
        shadow_map_destroy(&sh->map);
        return 0;
    }
    shadow_pcf_build(&sh->pcf, SHADOW_MAP_SIZE);
    // ~2 second time-constant on the sun slew. slow enough to kill crawl,
    // fast enough you dont notice the lag at sunrise.
    shadow_stabilize_init(&sh->stab, 0.5f);

    sh->ready = 1;
    LOGI("shadow: csm online (%d cascades, lambda %.2f)",
         SHADOW_CASCADE_COUNT, (double)SHADOW_SPLIT_LAMBDA);
    return 1;
}

void shadow_shutdown(shadow_system *sh) {
    if (!sh->ready) return;
    shadow_pass_shutdown(&sh->pass);
    shadow_map_destroy(&sh->map);
    sh->ready = 0;
}

void shadow_update(shadow_system *sh, const camera *cam, vec3 sun_dir, float dt) {
    if (!sh->ready) return;

    // sun_dir as passed points from the sun toward the ground already (that's
    // what sun_direction gives), but guard against a zero vector at the
    // horizon transition where it can briefly degenerate.
    if (vec3_length_sq(sun_dir) < 1e-6f) sun_dir = vec3_new(0.0f, -1.0f, 0.0f);

    // slew toward it so a slowly-moving sun doesnt make the edges shimmer.
    vec3 stable_dir = shadow_stabilize_dir(&sh->stab, sun_dir, dt);

    shadow_split_compute(&sh->csm, cam->znear, cam->zfar);

    mat4 view = camera_view(cam);
    shadow_matrix_update(&sh->csm, view, cam->fov, cam->aspect, stable_dir);

    // cull frusta depend on the matrices we just built.
    shadow_cull_build(&sh->cull, &sh->csm);
}

void shadow_render(shadow_system *sh, shadow_caster_fn draw, void *ctx) {
    if (!sh->ready) return;
    shadow_pass_render(&sh->pass, &sh->map, &sh->csm, draw, ctx);
}

void shadow_bind(shadow_system *sh, glid lighting_prog, int tex_unit) {
    if (!sh->ready) return;

    shadow_map_bind_sampler(&sh->map, tex_unit);
    gl_set_uniform_int(lighting_prog, "u_shadow_map", tex_unit);
    gl_set_uniform_int(lighting_prog, "u_shadow_count", sh->csm.count);
    gl_set_uniform_int(lighting_prog, "u_shadow_on", sh->csm.enabled);
    gl_set_uniform_float(lighting_prog, "u_shadow_max_dist", SHADOW_MAX_DISTANCE);
    gl_set_uniform_float(lighting_prog, "u_shadow_fade", SHADOW_FADE_BAND);

    shadow_pcf_upload(lighting_prog, &sh->pcf, &sh->csm);
}

void shadow_set_enabled(shadow_system *sh, int on) {
    sh->csm.enabled = on ? 1 : 0;
}

int shadow_caster_visible(const shadow_system *sh, int cascade, aabb box) {
    if (!sh->ready) return 1;   // no shadows up yet, dont skip draws
    return shadow_cull_test(&sh->cull, &sh->csm, cascade, box);
}
