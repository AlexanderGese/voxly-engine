#ifndef RENDER_VOLUMETRIC_TARGET_H
#define RENDER_VOLUMETRIC_TARGET_H

#include "../gl.h"

// the offscreen render targets the pass marches into. we keep two single-
// attachment RGBA16F buffers at reduced resolution: one the march writes, one
// the blur ping-pongs through. half (or quarter) res because the scatter is
// low frequency and we blur it anyway — marching at full res is a waste.
//
// kept deliberately dumb: no depth attachment (the march reads the gbuffer
// depth as a texture, it doesn't write depth), just colour.

typedef struct {
    glid fbo[2];
    glid tex[2];
    int  w, h;        // reduced resolution
    int  full_w, full_h;
    int  scale;       // full / reduced
} volumetric_target;

// allocate at full_w/scale x full_h/scale. returns 1 on success. a scale <= 0
// is treated as 1. always leaves fbo 0 bound.
int  volumetric_target_init(volumetric_target *t, int full_w, int full_h, int scale);

void volumetric_target_destroy(volumetric_target *t);

// resize to a new full resolution keeping the same scale. cheap no-op if the
// reduced dimensions wouldn't change.
void volumetric_target_resize(volumetric_target *t, int full_w, int full_h);

// bind fbo[which] (0 or 1) and set the viewport to the reduced res.
void volumetric_target_bind(const volumetric_target *t, int which);

#endif
