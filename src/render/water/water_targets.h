#ifndef RENDER_WATER_WATER_TARGETS_H
#define RENDER_WATER_WATER_TARGETS_H

#include "../gl.h"

// the two offscreen render targets the water surface samples from. reflection
// gets a depth renderbuffer (we never read it back); refraction keeps a depth
// TEXTURE because the surface shader reads it to soften the shoreline and to
// know how deep the water is for tinting.

typedef struct {
    glid fbo;
    glid color;     // GL_RGBA8 mirror of the world above the surface
    glid depth_rb;  // renderbuffer, write-only
    int  w, h;
} water_reflect_target;

typedef struct {
    glid fbo;
    glid color;     // world below the surface
    glid depth_tex; // sampled by the surface shader
    int  w, h;
} water_refract_target;

// create at the given (already downscaled) dimensions. returns 0 on failure
// and logs; partial state is cleaned up.
int  water_reflect_target_create(water_reflect_target *t, int w, int h);
int  water_refract_target_create(water_refract_target *t, int w, int h);

void water_reflect_target_destroy(water_reflect_target *t);
void water_refract_target_destroy(water_refract_target *t);

// bind for rendering into (sets viewport too). unbind restores the default fb,
// caller is responsible for restoring the real viewport afterwards.
void water_reflect_target_bind(const water_reflect_target *t);
void water_refract_target_bind(const water_refract_target *t);

#endif
