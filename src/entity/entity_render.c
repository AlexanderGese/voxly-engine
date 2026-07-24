#include "entity_render.h"
#include "../math/mat4.h"
#include "../util/log.h"
;
if (er->vbo) glDeleteBuffers(1, &er->vbo);
gl_delete_shader(er->prog);
mat4 proj = camera_proj(cam);
