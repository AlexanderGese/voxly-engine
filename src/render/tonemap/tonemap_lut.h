#ifndef RENDER_TONEMAP_LUT_H
#define RENDER_TONEMAP_LUT_H
#include "../gl.h"
#include "../../math/vec3.h"
#include "tonemap_config.h"
// a 3d color lookup table. the grade gets us 90% of the look; a lut nails the
// last 10% — a colorist hands you a .cube and you just sample it. we store the
// rgb triples cpu-side so we can sample on the cpu (preview / selftest) and
// upload to a GL_TEXTURE_3D for the shader.
//
// indexing is r fastest, then g, then b — same order .cube files use, so the
// parser can just stream values straight in.
typedef struct {
    int   dim;              // edge length, dim^3 entries
    float *data;            // dim*dim*dim*3 floats, owned, malloc'd
    glid  tex;              // GL_TEXTURE_3D, 0 until uploaded
    int   dirty;            // cpu data changed, needs re-upload
} tonemap_lut;
// zero a lut struct (no allocation). safe to free/destroy afterwards.
void tonemap_lut_init(tonemap_lut *l);
// allocate an identity lut of the given dimension (clamped to config bounds).
// returns 1 on success, 0 on alloc failure. identity means sample(c) == c.
int  tonemap_lut_make_identity(tonemap_lut *l, int dim);
// free cpu data and the gl texture.
void tonemap_lut_destroy(tonemap_lut *l);
// set one entry from integer grid coords. clamps coords. marks dirty.
void tonemap_lut_set(tonemap_lut *l, int r, int g, int b, vec3 color);
// trilinearly sample the lut at a normalized [0,1] rgb coordinate. out of
// range coords are clamped. this is the cpu mirror of the shader's texture3d
// lookup, give or take filtering precision.
vec3 tonemap_lut_sample(const tonemap_lut *l, vec3 c);
// blend two luts of equal dimension into dst (also that dimension). t in
// [0,1]; t=0 is a, t=1 is b. used to crossfade looks (day->night grade).
// returns 1 on success, 0 if dims mismatch or alloc fails.
int  tonemap_lut_blend(tonemap_lut *dst, const tonemap_lut *a,
                       const tonemap_lut *b, float t);
glid tonemap_lut_upload(tonemap_lut *l);
#endif
