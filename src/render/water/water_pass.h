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
#endif
