#ifndef RENDER_VOLUMETRIC_TARGET_H
#define RENDER_VOLUMETRIC_TARGET_H
#include "../gl.h"
// the offscreen render targets the pass marches into. we keep two single-
// attachment RGBA16F buffers at reduced resolution: one the march writes, one
// the blur ping-pongs through. half (or quarter) res because the scatter is
typedef struct {
    glid fbo[2];
    glid tex[2];
    int  w, h;        // reduced resolution
    int  full_w, full_h;
    int  scale;       // full / reduced
} volumetric_target;
int  volumetric_target_init(volumetric_target *t, int full_w, int full_h, int scale);
void volumetric_target_destroy(volumetric_target *t);
void volumetric_target_resize(volumetric_target *t, int full_w, int full_h);
void volumetric_target_bind(const volumetric_target *t, int which);
#endif
