#ifndef RENDER_SKYBOX_ATMOSPHERE_H
#define RENDER_SKYBOX_ATMOSPHERE_H

// the front of the render/skybox module. owns the palette, the baked star
// field and a dome, and every frame produces a snapshot the render layer (or
// anyone wanting sky/fog colors) can read. this is the thing main/renderer
// holds onto.
//
// it deliberately does NOT own a clock — the world/sky sim drives the hour and
// weather, we just consume them. keeps the look decoupled from the sim.

#include "skyb_common.h"
#include "skyb_palette.h"
#include "skyb_celestial.h"
#include "skyb_stars.h"
#include "skyb_gradient.h"
#include "skyb_fog.h"

// what the renderer needs to draw a frame of sky + fog the world.
typedef struct {
    skyb_gradient grad;        // dome gradient inputs (also feeds fog)
    skyb_fog      fog;         // distance fog tuned to the sky
    skyb_body     sun;
    skyb_body     moon;
    skyb_rgb      ambient;     // flat ambient light color for the scene
    float         star_vis;    // 0..1 star visibility this frame
    float         exposure;    // tonemap exposure for the final dome shade
    float         hour;        // echoed for convenience
} skyb_frame;

// tunables that don't change per frame.
typedef struct {
    skyb_palette   palette;
    skyb_starfield stars;
    skyb_dome      dome;

    float orbit_tilt;   // radians, sun/moon arc lean
    float dome_radius;  // world units
    float view_dist;    // far plane in blocks, for fog

    float time_s;       // wall-clock accumulator for twinkle/scatter
    skyb_frame frame;   // last baked snapshot
} skyb_atmosphere;

// set it all up. seed feeds the star bake; view_dist is the far plane in
// blocks. builds a default palette + dome you can tweak afterwards.
void skyb_atmosphere_init(skyb_atmosphere *a, unsigned seed, float view_dist);

// advance the wall-clock accumulator (twinkle/scatter animation only).
void skyb_atmosphere_tick(skyb_atmosphere *a, float dt);

// rebake the frame snapshot for the given in-game hour, moon phase (0..1) and
// wetness (0..1, from weather). also reshades the dome mesh.
void skyb_atmosphere_bake(skyb_atmosphere *a, float hour, float moon_phase,
                          float wetness);

// read the last baked frame.
const skyb_frame *skyb_atmosphere_frame(const skyb_atmosphere *a);

// the clear color to hand glClearColor (horizon-ish, so the seam under the
// dome skirt matches). already tonemapped.
skyb_rgb skyb_atmosphere_clear_color(const skyb_atmosphere *a);

void skyb_atmosphere_free(skyb_atmosphere *a);

#endif
