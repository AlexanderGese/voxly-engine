#include "dof_params.h"

void dof_params_defaults(dof_params *p) {
    p->enabled       = 1;
    p->strength      = DOFX_DEFAULT_STRENGTH;
    p->tap_count     = DOFX_KERNEL_DEFAULT;
    p->res_scale     = DOFX_DEFAULT_SCALE;
    p->near_dilate   = (float)DOFX_NEAR_DILATE;
    p->focus_epsilon = DOFX_FOCUS_EPSILON;
    p->debug_view    = DOF_VIEW_NORMAL;
}

void dof_params_sanitize(dof_params *p) {
    if (p->strength < 0.0f) p->strength = 0.0f;
    if (p->strength > 1.0f) p->strength = 1.0f;

    if (p->tap_count < 1)               p->tap_count = 1;
    if (p->tap_count > DOFX_KERNEL_MAX) p->tap_count = DOFX_KERNEL_MAX;

    // res_scale is a power-of-two-ish downscale. snap stray values to the
    // nearest of the three we actually support so the buffer math stays clean.
    if (p->res_scale <= 1)      p->res_scale = 1;
    else if (p->res_scale <= 3) p->res_scale = 2;
    else                        p->res_scale = 4;

    if (p->near_dilate < 0.0f)  p->near_dilate = 0.0f;
    if (p->near_dilate > DOFX_MAX_COC_TEXELS) p->near_dilate = DOFX_MAX_COC_TEXELS;

    if (p->focus_epsilon < 0.0f) p->focus_epsilon = 0.0f;

    if (p->debug_view < 0 || p->debug_view >= DOF_VIEW_COUNT)
        p->debug_view = DOF_VIEW_NORMAL;
}

int dof_params_cycle_view(dof_params *p) {
    p->debug_view = (p->debug_view + 1) % DOF_VIEW_COUNT;
    return p->debug_view;
}

int dof_params_needs_rebuild(const dof_params *p, const dof_params *old) {
    // only res and tap count touch gpu resources; everything else is uniforms.
    return p->res_scale != old->res_scale || p->tap_count != old->tap_count;
}
