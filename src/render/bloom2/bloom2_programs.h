#ifndef RENDER_BLOOM2_PROGRAMS_H
#define RENDER_BLOOM2_PROGRAMS_H
#include "../gl.h"
// the shader programs the chain needs. all four share the same fullscreen
typedef struct {
    glid bright;
    glid down;
    glid up;
    glid composite;
} bloom2_programs;
int  bloom2_programs_load(bloom2_programs *p);
void bloom2_programs_destroy(bloom2_programs *p);
int  bloom2_programs_ok(const bloom2_programs *p);
#endif
