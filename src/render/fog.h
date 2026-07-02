#ifndef RENDER_FOG_H
#define RENDER_FOG_H
#include "gl.h"
typedef struct {
    float near_dist;
    float far_dist;
    float r, g, b;
} fog_params;
#endif
