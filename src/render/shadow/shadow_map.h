#ifndef RENDER_SHADOW_MAP_H
#define RENDER_SHADOW_MAP_H

#include "shadow_config.h"
#include "../gl.h"

// the gpu side: one depth-only framebuffer over a 2d texture *array*, one
// layer per cascade. a layered fbo means we can bind a layer per depth pass
// without juggling N separate fbos.

typedef struct {
    glid fbo;
    glid depth_array;    // GL_TEXTURE_2D_ARRAY, depth component, COUNT layers
    int  size;           // per-layer resolution
    int  layers;
} shadow_map;

int  shadow_map_create(shadow_map *sm, int size);
void shadow_map_destroy(shadow_map *sm);

// bind the fbo and attach the given cascade layer as the depth target, then
// set the viewport + clear. leaves you ready to draw casters.
void shadow_map_begin_layer(shadow_map *sm, int layer);

// unbind back to the default framebuffer.
void shadow_map_end(const shadow_map *sm);

// bind the whole depth array to a texture unit for sampling in the main pass.
void shadow_map_bind_sampler(const shadow_map *sm, int unit);

#endif
