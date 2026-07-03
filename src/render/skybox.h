#ifndef RENDER_SKYBOX_H
#define RENDER_SKYBOX_H

#include "gl.h"
#include "camera.h"

// solid color gradient skybox. no cubemap — i was going to do one but
// a vertical gradient looks cleaner and is way less code

void skybox_init(void);
void skybox_draw(glid prog, const camera *cam);

#endif
