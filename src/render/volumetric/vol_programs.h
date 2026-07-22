#ifndef RENDER_VOLUMETRIC_PROGRAMS_H
#define RENDER_VOLUMETRIC_PROGRAMS_H

#include "../gl.h"

// the three gl programs the pass drives, plus their shared passthrough vert.
// loaded once at init; if any fail to compile we report it and the pass falls
// back to its disabled state (the cpu reference march still works for tests).

typedef struct {
    glid march;       // volumetric_march.frag — the raymarch
    glid blur;        // volumetric_blur.frag — separable bilateral blur
    glid composite;   // volumetric_composite.frag — additive over scene
    int  ok;          // 1 if all three linked
} volumetric_programs;

// compile/link all three. returns p->ok. on partial failure it deletes whatever
// did load so we don't leak half a pipeline.
int  volumetric_programs_load(volumetric_programs *p);

void volumetric_programs_destroy(volumetric_programs *p);

// bind helpers that also pin the standard sampler uniforms to their fixed units
// (see VOL_TEX_UNIT_* in vol_config.h). saves repeating the glUniform1i dance.
void volumetric_programs_use_march(const volumetric_programs *p);
void volumetric_programs_use_blur(const volumetric_programs *p);
void volumetric_programs_use_composite(const volumetric_programs *p);

#endif
