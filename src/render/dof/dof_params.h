#ifndef RENDER_DOF_PARAMS_H
#define RENDER_DOF_PARAMS_H
#include "dof_config.h"
// the runtime-tweakable knobs for the dof chain, separated from the lens
// optics (dof_lens) because these are presentation choices — how strong, how
// many taps, half-res or not — rather than physics. the debug ui pokes this
// struct live; nothing here needs a rebuild to take effect except `res_scale`
// and `tap_count`, which the manager watches for changes.
typedef struct {
    int   enabled;       // master switch. 0 = pure passthrough.
    float strength;      // 0..1 final blend of the blurred result over sharp
    int   tap_count;     // bokeh taps, clamped to DOFX_KERNEL_MAX
    int   res_scale;     // 1/2/4 downscale of the gather buffers
    float near_dilate;   // texels the near coc dilates before composite
    float focus_epsilon; // coc below this (texels) counts as in-focus
    int   debug_view;    // 0 normal, 1 show coc, 2 near, 3 far (see enum)
} dof_params;
// view modes for debug_view. handy for eyeballing whether the coc sign and
// magnitude are sane before you trust the composite.
enum {
    DOF_VIEW_NORMAL = 0,
    DOF_VIEW_COC,
    DOF_VIEW_NEAR,
    DOF_VIEW_FAR,
    DOF_VIEW_COUNT
}
;
// load defaults from config.h.
void dof_params_defaults(dof_params *p);
// clamp every field into its legal range. call after the ui mangles it. the
// manager also calls this so out-of-range values from a save file cant
// explode the gather.
void dof_params_sanitize(dof_params *p);
// step debug_view to the next mode, wrapping. returns the new mode.
int  dof_params_cycle_view(dof_params *p);
int  dof_params_needs_rebuild(const dof_params *p, const dof_params *old);
#endif
