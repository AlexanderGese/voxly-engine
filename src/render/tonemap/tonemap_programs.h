#ifndef RENDER_TONEMAP_PROGRAMS_H
#define RENDER_TONEMAP_PROGRAMS_H

#include "../gl.h"

// shader programs for the tonemap pass. there's really only one — the
// fullscreen grade shader — but i keep the loader in its own unit to match the
// other post modules and to leave room for a debug "split view" variant.

typedef struct {
    glid grade;    // the main hdr -> ldr tonemap + grade + lut shader
    glid split;    // optional before/after split-screen debug shader
} tonemap_programs;

// load the shaders. returns 1 if the main grade program compiled (split is
// optional). non-fatal: a missing shader just disables the pass.
int  tonemap_programs_load(tonemap_programs *p);
void tonemap_programs_destroy(tonemap_programs *p);

// is the pass usable? (grade program present.)
int  tonemap_programs_ok(const tonemap_programs *p);

#endif
