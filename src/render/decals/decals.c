#include "decals.h"
#include "decals_box.h"
#include "decals_config.h"
#include "../../config.h"
#include "../../util/log.h"

#include <math.h>
#include <string.h>

int decals_init(decals_system *s) {
    memset(s, 0, sizeof *s);
    decals_pool_init(&s->pool);
    decals_atlas_init(&s->atlas);
    s->view_proj     = mat4_identity();
    s->inv_view_proj = mat4_identity();
    s->cam_pos       = VEC3_ZERO;
    s->max_dist      = (float)(RENDER_DISTANCE * CHUNK_SIZE_X);
    s->ready         = 0;

    if (!decals_pass_init(&s->pass)) {
        LOGW("decals: pass init failed, decals disabled this run");
        return 0;
    }
    s->ready = 1;
    LOGI("decals: system ready (cap %d, cull dist %.0f)", DECALS_MAX, s->max_dist);
    return 1;
}

void decals_shutdown(decals_system *s) {
    if (s->ready) decals_pass_shutdown(&s->pass);
    s->ready = 0;
}

void decals_set_atlas_texture(decals_system *s, glid atlas_tex) {
    decals_atlas_set_texture(&s->atlas, atlas_tex);
    if (s->atlas.count == 0)
        decals_atlas_load_defaults(&s->atlas);
}

void decals_update(decals_system *s, float dt) {
    if (dt < 0.0f) dt = 0.0f;
    decals_pool_tick(&s->pool, dt);
}

void decals_set_camera(decals_system *s, mat4 view_proj, mat4 inv_view_proj,
                       vec3 cam_pos, int screen_w, int screen_h) {
    s->view_proj     = view_proj;
    s->inv_view_proj = inv_view_proj;
    s->cam_pos       = cam_pos;
    decals_pass_set_camera(&s->pass, view_proj, inv_view_proj);
    decals_pass_set_screen(&s->pass, screen_w, screen_h);
}

void decals_set_max_dist(decals_system *s, float d) {
    if (d > 0.0f) s->max_dist = d;
}

int decals_render(decals_system *s, glid depth_tex, glid gnormal_tex) {
    if (!s->ready) return 0;
    if (decals_pool_live_count(&s->pool) == 0) return 0;

    decals_cull_gather(&s->vis, &s->pool, s->view_proj, s->cam_pos, s->max_dist);
    if (s->vis.count == 0) return 0;

    return decals_pass_draw(&s->pass, &s->pool, &s->vis, &s->atlas,
                            depth_tex, gnormal_tex);
}

// resolve a named region and stamp its rects + flags into the descriptor, then
// hand off to the pool. the pool owns slot allocation and fade setup.
decals_handle decals_spawn_named(decals_system *s, const char *region_name,
                                 decals_spawn_desc *desc) {
    int idx = decals_atlas_find(&s->atlas, region_name);
    if (idx < 0) {
        LOGW("decals: unknown region '%s'", region_name);
        return DECALS_INVALID_HANDLE;
    }
    const decals_atlas_region *r = decals_atlas_region_at(&s->atlas, idx);
    desc->region     = *r;
    desc->has_region = 1;
    // fold the atlas region's flags into the spawn (normal-map, additive, etc).
    desc->flags |= decals_atlas_flags_at(&s->atlas, idx);

    return decals_pool_spawn(&s->pool, desc);
}

// rotate a vector around an axis by `rad` (rodrigues). used to jitter the up
// hint so stamped decals dont all share the exact same orientation.
static vec3 rotate_about(vec3 v, vec3 axis, float rad) {
    float c = cosf(rad), sn = sinf(rad);
    vec3 t1 = vec3_scale(v, c);
    vec3 t2 = vec3_scale(vec3_cross(axis, v), sn);
    vec3 t3 = vec3_scale(axis, vec3_dot(axis, v) * (1.0f - c));
    return vec3_add(vec3_add(t1, t2), t3);
}

decals_handle decals_stamp(decals_system *s, const char *region_name,
                           vec3 hit, vec3 surf_normal,
                           float size, float depth, float rotation) {
    if (size < DECALS_MIN_HALF_EXTENT) {
        LOGW("decals: stamp '%s' too small (%.3f), ignoring", region_name, size);
        return DECALS_INVALID_HANDLE;
    }

    vec3 n = vec3_normalize(surf_normal);
    // projector forward looks *into* the surface, i.e. along -normal. push the
    // box center a hair off the surface along the normal so the volume straddles
    // it (half its depth in front, half behind) and the depth test in the fs has
    // slop to work with.
    vec3 fwd = vec3_neg(n);
    vec3 center = vec3_add(hit, vec3_scale(n, depth * 0.5f));

    // up hint: anything not parallel to the normal, then jittered by rotation so
    // repeated stamps on the same wall dont visibly tile.
    vec3 up_hint = (fabsf(n.y) > 0.9f) ? VEC3_FWD : VEC3_UP;
    up_hint = rotate_about(up_hint, n, rotation);

    decals_spawn_desc desc;
    decals_spawn_desc_defaults(&desc);
    desc.proj = decals_box_make(center, fwd, up_hint,
                                vec3_new(size, size, depth));
    desc.angle_fade = DECALS_DEFAULT_ANGLE_FADE;

    return decals_spawn_named(s, region_name, &desc);
}

void decals_remove(decals_system *s, decals_handle h) {
    decals_pool_kill(&s->pool, h);
}

int decals_count(const decals_system *s) {
    return decals_pool_live_count(&s->pool);
}
