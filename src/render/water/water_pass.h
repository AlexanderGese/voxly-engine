#ifndef RENDER_WATER_WATER_PASS_H
#define RENDER_WATER_WATER_PASS_H

#include "../gl.h"
#include "../camera.h"
#include "../../world/world.h"
#include "water_plane.h"
#include "water_wave.h"
#include "water_targets.h"
#include "water_surface_mesh.h"
#include "water_caustics.h"

// the whole water render pass, wired up. owns the reflection/refraction
// targets, the wave field, the surface mesh and the caustic projector, plus
// the surface shader. usage per frame:
//
// water_pass_begin(p, cam);                 // compute mirrored view, etc
// water_pass_render_reflection(p, world);   // -> calls your world draw cb
// water_pass_render_refraction(p, world);
// water_pass_end(p);                        // draw the surface to screen
//
// the world-draw callback lets us reuse the existing chunk renderer for the
// off-screen passes without this module knowing how chunks are drawn.

// signature: render the world with the given view/proj and clip plane.
typedef void (*water_world_draw_fn)(void *user, const float *view,
                                    const float *proj, const float *clip_eq);

typedef struct {
    water_plane          plane;
    water_wave_field     waves;
    water_reflect_target reflect;
    water_refract_target refract;
    water_surface_mesh   mesh;
    water_caustics       caustics;

    glid prog;          // the surface shader (reflection+refraction+fresnel)

    water_world_draw_fn  draw_cb;
    void                *draw_user;

    int   screen_w, screen_h;
    float surface_y;
    int   enabled;
} water_pass;

// init at screen resolution; surface sits at world height surface_y (sea level
// by default). seed feeds the wave field. returns 0 on hard failure.
int  water_pass_init(water_pass *p, int screen_w, int screen_h,
                     float surface_y, unsigned seed);
void water_pass_destroy(water_pass *p);

// hook up how the world gets drawn for the offscreen passes.
void water_pass_set_world_draw(water_pass *p, water_world_draw_fn fn, void *user);

// rebuild the offscreen targets on window resize.
void water_pass_resize(water_pass *p, int screen_w, int screen_h);

// advance wave + caustic animation.
void water_pass_tick(water_pass *p, float dt);

// the three render phases. begin must run before the two render_* calls.
// the render_* calls take the live camera so they can build the mirrored /
// clipped views and invoke the world-draw callback into the offscreen targets.
void water_pass_begin(water_pass *p, const camera *cam);
void water_pass_render_reflection(water_pass *p, const camera *cam);
void water_pass_render_refraction(water_pass *p, const camera *cam);

// draw the lit water surface to the currently-bound framebuffer. needs the
// real (unmirrored) camera and the current time for the shader ripple.
void water_pass_end(water_pass *p, const camera *cam, float time);

#endif
