#ifndef RENDER_DOF_STATS_H
#define RENDER_DOF_STATS_H

#include "dof_coc.h"
#include "dof_kernel.h"

// little introspection helpers for the debug overlay. nothing here is on the
// hot path; it's the stuff you want when the dof "looks wrong" and you need to
// know whether the coc is sane, where the focal plane is, and how the bokeh
// disc is distributed. all cpu, no gl.

// a coarse histogram of signed coc across a sampled depth range. negative bins
// are near field, positive are far, the middle bin straddles the focal plane.
#define DOF_HIST_BINS  9

typedef struct {
    int   bins[DOF_HIST_BINS]; // sample counts per bin
    float min_coc;             // smallest (most-near) signed coc seen
    float max_coc;             // largest (most-far) signed coc seen
    int   in_focus;            // count below the focus epsilon
    int   total;               // samples taken
} dof_stats;

// sweep `count` distances log-spaced from `near` to `far`, evaluate the lens
// coc at each, and bin the result. handy to eyeball the near/far balance for a
// given aperture. zeroes `s` first.
void  dof_stats_sweep(dof_stats *s, const dof_lens *lens,
                      float near, float far, int count, float epsilon);

// the hyperfocal distance: the focus distance beyond which everything to
// infinity is acceptably sharp. classic photography number, also a sane upper
// bound for the autofocus ceiling. derived from the lens + a chosen coc limit.
float dof_stats_hyperfocal(const dof_lens *lens, float coc_limit_texels,
                           float texel_scale);

// uniformity metric for a bokeh kernel: the ratio of the densest local cell to
// the sparsest in a coarse grid over the unit disc. ~1.0 means evenly packed
// (good), large means the spiral degenerated. tests pin this under a ceiling.
float dof_stats_kernel_uniformity(const dof_kernel *k);

// format a one-line summary into `buf` (e.g. for an on-screen readout).
// returns the length written, never overflows `cap`.
int   dof_stats_format(const dof_stats *s, char *buf, int cap);

#endif
