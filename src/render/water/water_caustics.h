#ifndef RENDER_WATER_WATER_CAUSTICS_H
#define RENDER_WATER_WATER_CAUSTICS_H

#include "../gl.h"

// fake caustics. instead of tracing photons we keep a small animated tiling
// texture (a voronoi-ish ripple) that gets projected straight down onto
// whatever is under the water, modulated by depth. it reads as "light dancing
// on the lakebed" and costs basically nothing.

typedef struct {
    glid tex;       // the animated caustic pattern, GL_R8
    int  size;      // texels per side
    float phase;    // animation accumulator
    unsigned char *cpu;  // scratch buffer we re-bake into each frame
} water_caustics;

// allocate a size x size pattern. size should be a power of two-ish.
int  water_caustics_create(water_caustics *c, int size);
void water_caustics_destroy(water_caustics *c);

// advance + re-bake the pattern. cheap enough to do a few times a second;
// caller can skip frames if it wants. returns the gl texture id for binding.
void water_caustics_tick(water_caustics *c, float dt);

// sample the (cpu copy of the) pattern at uv, bilinear, 0..1. exposed so the
// shoreline foam pass can reuse the same field for sparkle.
float water_caustics_sample(const water_caustics *c, float u, float v);

void water_caustics_bind(const water_caustics *c, int unit);

#endif
