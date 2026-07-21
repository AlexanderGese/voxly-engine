#ifndef RENDER_TONEMAP_PROGRAMS_H
#define RENDER_TONEMAP_PROGRAMS_H
#include "../gl.h"
typedef struct {
    glid grade;    // the main hdr -> ldr tonemap + grade + lut shader
    glid split;    // optional before/after split-screen debug shader
} tonemap_programs;
#endif
