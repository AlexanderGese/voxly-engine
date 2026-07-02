#ifndef RENDER_FOG_H
#define RENDER_FOG_H

#include "gl.h"

// fog uniform block. just a helper to push 3 floats into a shader program.
// fog ranges are in world units.

typedef struct {
    float near_dist;
    float far_dist;
    float r, g, b;
} fog_params;

void fog_defaults(fog_params *f);
void fog_apply(glid prog, const fog_params *f);

#endif
