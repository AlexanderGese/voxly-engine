#ifndef RENDER_DOF_KERNEL_H
#define RENDER_DOF_KERNEL_H

#include "dof_config.h"
#include "../../math/vec2.h"

// the bokeh gather kernel: a disk of tap offsets the gather pass smears each
// fragment across. laid out as a vogel/sunflower spiral (golden angle) so the
// samples are near-uniform and dont collapse into the visible rings a naive
// concentric pattern gives at low tap counts.
//
// offsets are unit-disk normalized (radius <= 1); the gather scales them by
// the per-fragment coc in texels. we keep the cpu copy so a test can assert
// the disk is actually a disk and the shader has a reference to upload.

typedef struct {
    vec2 offsets[DOFX_KERNEL_MAX]; // unit-disk tap positions
    float weight[DOFX_KERNEL_MAX]; // per-tap weight (rim-biased for bokeh)
    int   count;
} dof_kernel;

// build `count` vogel-spiral taps. count is clamped to [1, DOFX_KERNEL_MAX].
// returns the count actually generated.
int   dof_kernel_build(dof_kernel *k, int count);

// the golden angle in radians (~137.5 deg). exposed so tests can pin the
// layout math.
float dof_kernel_golden_angle(void);

// total of all tap weights, for normalizing the gather sum. recomputed rather
// than cached so it stays honest if someone pokes the weights.
float dof_kernel_weight_sum(const dof_kernel *k);

// pack offsets into a tight float array (count*2 floats) for glUniform2fv.
// `out` must hold count*2 floats. returns floats written.
int   dof_kernel_pack(const dof_kernel *k, float *out);

// largest radius any tap reaches. should be ~1 for a full disk; used to scale
// the coc into the same space the kernel lives in.
float dof_kernel_max_radius(const dof_kernel *k);

#endif
