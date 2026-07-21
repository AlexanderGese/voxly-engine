#ifndef RENDER_TONEMAP_LUT_H
#define RENDER_TONEMAP_LUT_H
#include "../gl.h"
#include "../../math/vec3.h"
#include "tonemap_config.h"
typedef struct {
    int   dim;              // edge length, dim^3 entries
    float *data;            // dim*dim*dim*3 floats, owned, malloc'd
    glid  tex;              // GL_TEXTURE_3D, 0 until uploaded
    int   dirty;            // cpu data changed, needs re-upload
} tonemap_lut;
void tonemap_lut_init(tonemap_lut *l);
#endif
