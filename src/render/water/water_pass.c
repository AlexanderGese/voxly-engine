#include "water_pass.h"
#include "water_config.h"
#include "water_reflcam.h"
#include "../../util/log.h"
#include <math.h>
#include <stddef.h>
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
water_caustics_create(&p->caustics, 128);
p->prog = gl_load_shader("src/render/water/shaders/water_surface.vert",
                             "src/render/water/shaders/water_surface.frag");
p->draw_user = user;
water_caustics_tick(&p->caustics, dt);
water_view v = water_reflcam_reflection(cam, &p->plane);
water_reflect_target_bind(&p->reflect);
glClearColor(0, 0, 0, 1);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glFrontFace(GL_CW);
p->draw_cb(p->draw_user, mat4_data(&v.view), mat4_data(&v.proj),
               (const float*)&v.clip_eq);
glFrontFace(GL_CCW);
glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
glEnable(GL_DEPTH_TEST);
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
water_surface_mesh_draw(&p->mesh);
glDisable(GL_BLEND);
glActiveTexture(GL_TEXTURE0);
}
