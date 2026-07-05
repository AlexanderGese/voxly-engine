#ifndef RENDER_DECALS_PROGRAM_H
#define RENDER_DECALS_PROGRAM_H

#include "../gl.h"

// the decal pass program: compiled from the inline source in decals_glsl, with
// its uniform locations cached up front so the per-batch draw isnt doing
// glGetUniformLocation in a hot loop. one program, no variants.

typedef struct {
    glid prog;          // 0 if compile/link failed

    // cached uniform locations, -1 if the driver optimised one out.
    int u_view_proj;
    int u_inv_view_proj;
    int u_screen;
    int u_depth;
    int u_gnormal;
    int u_atlas;
    int u_tint;
    int u_normal_strength;
} decals_program;

// compile + link from the inline glsl. returns 1 on success, 0 (and logs) on
// failure; on failure prog is 0 and the pass should no-op.
int  decals_program_build(decals_program *p);

void decals_program_destroy(decals_program *p);

// bind the program and set the sampler units (depth=0, gnormal=1, atlas=2).
// the per-frame matrices/scalars are pushed by the pass.
void decals_program_use(const decals_program *p);

#endif
