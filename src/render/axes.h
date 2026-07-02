#ifndef RENDER_AXES_H
#define RENDER_AXES_H

#include "gl.h"
#include "camera.h"

// debug gizmo: x/y/z axes at origin. colored red/green/blue.

typedef struct { glid vao, vbo, prog; } axes_gizmo;

int  axes_init(axes_gizmo *a);
void axes_destroy(axes_gizmo *a);
void axes_draw(axes_gizmo *a, const camera *cam, float scale);

#endif
