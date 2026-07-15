#ifndef RENDER_SHADOW_PASS_H
#define RENDER_SHADOW_PASS_H
#include "shadow_types.h"
#include "shadow_map.h"
typedef struct {
    glid prog_depth;     // the depth-only shader (position in, no color out)
    glid u_view_proj;    // cached uniform location
    int  cull_front;     // render back faces into the map to fight peter pan
} shadow_pass;
#endif
