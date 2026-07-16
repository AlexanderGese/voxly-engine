#ifndef RENDER_SKYBOX_GRADIENT_H
#define RENDER_SKYBOX_GRADIENT_H

// the sky dome gradient. we evaluate a vertical gradient (horizon -> zenith)
// plus a sun-direction scatter glow, and bake it onto a tessellated dome mesh
// so we can draw it as plain colored triangles. no shader trickery required,
// which keeps it portable to the engine's dumb fixed pipeline-ish setup.

#include "skyb_common.h"
#include "skyb_scatter.h"

// one dome vertex: position dir (unit, *radius applied at upload) + baked rgb.
typedef struct {
    float x, y, z;
    float r, g, b;
} skyb_dome_vertex;

// the dome is a uv-sphere cap. rings = vertical slices, sectors = around.
// upper hemisphere + a short skirt below the horizon so fog has room.
typedef struct {
    skyb_dome_vertex *verts;   // tri list, owned
    int               count;
    int               cap;
    int               rings;
    int               sectors;
    float             radius;
} skyb_dome;

// inputs that drive the gradient color at a given dome direction.
typedef struct {
    skyb_rgb zenith;
    skyb_rgb horizon;
    skyb_rgb sun_tint;
    vec3     sun_dir;     // unit
    float    sun_light;   // 0..1, scales the scatter glow
    float    haze;        // 0..1 extra horizon lift (weather/dust)
    int      use_scatter; // 1 = fold in the analytic scatter term below
    skyb_scatter scatter; // analytic scatter params (used iff use_scatter)
} skyb_gradient;

// the color the dome should be at a given view dir. pure function, also used
// by fog to sample the sky behind distant geometry.
skyb_rgb skyb_gradient_eval(const skyb_gradient *g, vec3 dir);

// allocate + tessellate the dome geometry. call once; rebake colors per frame.
void skyb_dome_build(skyb_dome *d, int rings, int sectors, float radius);

// recompute every vertex color from the current gradient. cheap enough to do
// each frame for a few hundred verts; cache if you get greedy.
void skyb_dome_shade(skyb_dome *d, const skyb_gradient *g);

void skyb_dome_free(skyb_dome *d);

#endif
