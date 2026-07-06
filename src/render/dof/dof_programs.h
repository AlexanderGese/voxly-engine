#ifndef RENDER_DOF_PROGRAMS_H
#define RENDER_DOF_PROGRAMS_H
#include "../gl.h"
#include "dof_kernel.h"
// the three gl programs the chain runs, plus their resolved uniform locations.
// loading is best-effort: if a .frag is missing we leave that program at 0 and
// flag `ok = 0`. the manager then falls back to a passthrough so the game
// still renders — dof is a luxury, not load-bearing.
//
// 1. coc      : depth -> signed circle-of-confusion buffer
// 2. gather   : color + coc -> separate near/far bokeh buffers (one draw,
// two-ish outputs handled by the manager binding the right fbo)
// 3. composite: sharp color + near + far -> final, near bleeding over sharp
typedef struct {
    glid coc;
    glid gather;
    glid composite;
    int  ok;          // 1 if all three compiled

    // cached uniform locations, looked up once at load. -1 if absent.
    struct {
        int focus_dist, focal_len, fstop, sensor_w;
        int max_coc, texel_scale, near_far;
    } u_coc;
    struct {
        int tex_color, tex_coc, taps, tap_count, texel_size, near_pass;
    } u_gather;
    struct {
        int tex_color, tex_near, tex_far, strength, near_dilate, debug_view;
    } u_comp;
} dof_programs;
// compile all three from the config paths, relative to the shader dir. returns
// 1 if every program built, 0 otherwise (and sets ok accordingly).
int  dof_programs_load(dof_programs *p);
// drop the gl programs. safe on a half-loaded struct.
void dof_programs_unload(dof_programs *p);
// push the static kernel taps into the gather program's uniform array. call
// once after a (re)build, not per frame. no-op if the program is missing.
void dof_programs_upload_kernel(const dof_programs *p, const dof_kernel *k);
#endif
