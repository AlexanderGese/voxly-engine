#ifndef RENDER_FXAA_PROGRAMS_H
#define RENDER_FXAA_PROGRAMS_H

#include "../gl.h"

// the two shader programs the pass needs. both share the post passthrough
// vertex shader. the prepass writes luma into alpha; the main pass does the
// edge filtering. they're split so the prepass can run at the same time we
// copy the scene through and the main pass stays a pure read of one texture.

typedef struct {
    glid prepass;   // scene -> rgba8 with luma in .a
    glid main;      // luma-edge filter -> backbuffer
} fxaa_programs;

// load both. returns 1 only if both compiled+linked. on partial failure it
// logs and returns 0 but leaves whatever loaded so destroy is safe.
int  fxaa_programs_load(fxaa_programs *p);
void fxaa_programs_destroy(fxaa_programs *p);

// both programs present?
int  fxaa_programs_ok(const fxaa_programs *p);

#endif
