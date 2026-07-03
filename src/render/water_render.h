#ifndef RENDER_WATER_RENDER_H
#define RENDER_WATER_RENDER_H

#include "gl.h"
#include "camera.h"
#include "texture.h"
#include "../world/world.h"

// separate rendering pass for water blocks with transparency + wave animation.
// drawn AFTER opaque blocks with blending enabled.

typedef struct {
    glid prog;
} water_renderer;

int  water_renderer_init(water_renderer *wr);
void water_renderer_destroy(water_renderer *wr);
void water_renderer_draw(water_renderer *wr, world *w, const camera *cam, float time);

#endif
