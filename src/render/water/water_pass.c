#include "water_pass.h"
#include "water_config.h"
#include "water_reflcam.h"
#include "../../util/log.h"

#include <math.h>
#include <stddef.h>

// how big a slice of water to tessellate around the player. covers a bit more
// than the render distance so the far edge meets the fog.
#define WATER_GRID_RES   48
#define WATER_GRID_CELL  2.0f

static int rtt_dim(int v) {
    int d = v / WATER_RTT_DOWNSCALE;
    return d < WATER_RTT_MIN_DIM ? WATER_RTT_MIN_DIM : d;
}

int water_pass_init(water_pass *p, int screen_w, int screen_h,
                    float surface_y, unsigned seed) {
    p->screen_w  = screen_w;
    p->screen_h  = screen_h;
    p->surface_y = surface_y;
    p->enabled   = 1;
    p->draw_cb   = NULL;
    p->draw_user = NULL;

    p->plane = water_plane_make(surface_y);
    water_wave_field_init(&p->waves, seed);

    int rw = rtt_dim(screen_w);
    int rh = rtt_dim(screen_h);
    if (!water_reflect_target_create(&p->reflect, rw, rh)) return 0;
    if (!water_refract_target_create(&p->refract, rw, rh)) {
        water_reflect_target_destroy(&p->reflect);
        return 0;
    }

    water_surface_mesh_create(&p->mesh, WATER_GRID_RES, WATER_GRID_CELL);
    water_caustics_create(&p->caustics, 128);

    // surface shaders live alongside this module rather than in the global
    // shaders/ dir — keeps the water pass self-contained.
    p->prog = gl_load_shader("src/render/water/shaders/water_surface.vert",
                             "src/render/water/shaders/water_surface.frag");
    if (!p->prog) {
        // missing shader is non-fatal — the rest of the engine still runs,
        // we just dont draw the surface. consistent with how bloom behaves.
        LOGW("water surface shader missing — water surface disabled");
        p->enabled = 0;
    }
    return 1;
}

void water_pass_destroy(water_pass *p) {
    water_reflect_target_destroy(&p->reflect);
    water_refract_target_destroy(&p->refract);
    water_surface_mesh_destroy(&p->mesh);
    water_caustics_destroy(&p->caustics);
    gl_delete_shader(p->prog);
    p->prog = 0;
}

void water_pass_set_world_draw(water_pass *p, water_world_draw_fn fn, void *user) {
    p->draw_cb   = fn;
    p->draw_user = user;
}

void water_pass_resize(water_pass *p, int screen_w, int screen_h) {
    if (p->screen_w == screen_w && p->screen_h == screen_h) return;
    p->screen_w = screen_w;
    p->screen_h = screen_h;

    int rw = rtt_dim(screen_w);
    int rh = rtt_dim(screen_h);
    water_reflect_target_destroy(&p->reflect);
    water_refract_target_destroy(&p->refract);
    water_reflect_target_create(&p->reflect, rw, rh);
    water_refract_target_create(&p->refract, rw, rh);
}

void water_pass_tick(water_pass *p, float dt) {
    water_wave_field_tick(&p->waves, dt);
    water_caustics_tick(&p->caustics, dt);
}

void water_pass_begin(water_pass *p, const camera *cam) {
    // keep the surface mesh under the camera. surface stays flat at surface_y;
    // the wave field supplies the displacement.
    water_surface_mesh_update(&p->mesh, &p->waves,
                              cam->pos.x, cam->pos.z, p->surface_y);
}

void water_pass_render_reflection(water_pass *p, const camera *cam) {
    if (!p->draw_cb || !p->enabled) return;
    water_view v = water_reflcam_reflection(cam, &p->plane);

    water_reflect_target_bind(&p->reflect);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // reflection flips winding — front faces become back faces under the mirror
    glFrontFace(GL_CW);
    p->draw_cb(p->draw_user, mat4_data(&v.view), mat4_data(&v.proj),
               (const float*)&v.clip_eq);
    glFrontFace(GL_CCW);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void water_pass_render_refraction(water_pass *p, const camera *cam) {
    if (!p->draw_cb || !p->enabled) return;
    water_view v = water_reflcam_refraction(cam, &p->plane);

    water_refract_target_bind(&p->refract);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    p->draw_cb(p->draw_user, mat4_data(&v.view), mat4_data(&v.proj),
               (const float*)&v.clip_eq);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void water_pass_end(water_pass *p, const camera *cam, float time) {
    if (!p->enabled) return;

    mat4 view = camera_view(cam);
    mat4 proj = camera_proj(cam);

    glViewport(0, 0, p->screen_w, p->screen_h);
    glUseProgram(p->prog);

    gl_set_uniform_mat4(p->prog, "u_view", mat4_data(&view));
    gl_set_uniform_mat4(p->prog, "u_proj", mat4_data(&proj));
    gl_set_uniform_vec3(p->prog, "u_cam_pos", cam->pos.x, cam->pos.y, cam->pos.z);
    gl_set_uniform_float(p->prog, "u_time", time);
    gl_set_uniform_float(p->prog, "u_distort", WATER_DISTORT_STRENGTH);
    gl_set_uniform_float(p->prog, "u_f0", WATER_F0);
    gl_set_uniform_float(p->prog, "u_fresnel_power", WATER_FRESNEL_POWER);
    gl_set_uniform_float(p->prog, "u_tint_depth", WATER_TINT_DEPTH);
    gl_set_uniform_vec3(p->prog, "u_tint_shallow",
                        WATER_TINT_SHALLOW_R, WATER_TINT_SHALLOW_G, WATER_TINT_SHALLOW_B);
    gl_set_uniform_vec3(p->prog, "u_tint_deep",
                        WATER_TINT_DEEP_R, WATER_TINT_DEEP_G, WATER_TINT_DEEP_B);

    // bind reflection (0), refraction color (1), refraction depth (2), caustics(3)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, p->reflect.color);
    gl_set_uniform_int(p->prog, "u_reflection", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, p->refract.color);
    gl_set_uniform_int(p->prog, "u_refraction", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, p->refract.depth_tex);
    gl_set_uniform_int(p->prog, "u_refract_depth", 2);

    water_caustics_bind(&p->caustics, 3);
    gl_set_uniform_int(p->prog, "u_caustics", 3);

    // water surfaces dont need to write depth from both sides; cull backfaces
    // and let the surface blend over the scene.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    water_surface_mesh_draw(&p->mesh);

    glDisable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);
}
