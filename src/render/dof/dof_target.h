#ifndef RENDER_DOF_TARGET_H
#define RENDER_DOF_TARGET_H

#include "../gl.h"

// a half-res (or quarter-res) color target the gather renders into. rgba16f
// so the bokeh sum keeps hdr highlights — the whole point of nice bokeh is the
// bright spots blooming into discs, and that dies if you clamp to 8-bit early.
//
// the dof chain uses three of these: the coc buffer, the near gather, and the
// far gather. they share this one type; only the size differs.

typedef struct {
    glid fbo;
    glid tex;
    int  w, h;
} dof_target;

// create a target at w x h. returns 0 on failure and leaves the struct zeroed
// so destroy is always safe.
int  dof_target_create(dof_target *t, int w, int h);
void dof_target_destroy(dof_target *t);

// bind for rendering and match the viewport.
void dof_target_bind(const dof_target *t);

// bind the color texture to a sampler unit; returns the unit for the uniform.
int  dof_target_bind_tex(const dof_target *t, int unit);

// resize in place. early-outs if the size is unchanged. returns 1 on success.
int  dof_target_resize(dof_target *t, int w, int h);

#endif
