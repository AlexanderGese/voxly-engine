#ifndef RENDER_BLOOM2_PROGRAMS_H
#define RENDER_BLOOM2_PROGRAMS_H

#include "../gl.h"

// the shader programs the chain needs. all four share the same fullscreen
// passthrough vertex shader; only the fragment stage differs.
// bright     - threshold/knee bright pass, src = scene
// down       - 13-tap karis downsample, src = prev mip
// up         - 9-tap tent upsample, additive into next-up mip
// composite  - final additive blend of mip[0] over the scene
//
// any program that fails to load is left as 0; the manager treats a zero
// bright program as "bloom unavailable" and bails gracefully.

typedef struct {
    glid bright;
    glid down;
    glid up;
    glid composite;
} bloom2_programs;

int  bloom2_programs_load(bloom2_programs *p);
void bloom2_programs_destroy(bloom2_programs *p);

// true only if every program loaded. the manager checks this once.
int  bloom2_programs_ok(const bloom2_programs *p);

#endif
