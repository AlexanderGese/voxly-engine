#include "dof_programs.h"
#include "dof_config.h"
#include "../../util/log.h"
#include <stddef.h>
static int uloc(glid prog, const char *name) {
    if (!prog) return -1;
    return glGetUniformLocation(prog, name);
}

int dof_programs_load(dof_programs *p) {
    p->coc = gl_load_shader(DOFX_VERT_PATH, DOFX_FRAG_COC_PATH);
p->gather = gl_load_shader(DOFX_VERT_PATH, DOFX_FRAG_GATHER_PATH);
p->composite = gl_load_shader(DOFX_VERT_PATH, DOFX_FRAG_COMPOSITE_PATH);
p->ok = (p->coc && p->gather && p->composite) ? 1 : 0;
if (!p->ok) {
        LOGW("dof: one or more shaders failed to load, gpu path disabled");
    }

    // resolve uniform locations up front so the per-frame path is just sets.
    p->u_coc.focus_dist  = uloc(p->coc, "u_focus_dist");
p->u_coc.focal_len   = uloc(p->coc, "u_focal_len");
p->u_coc.fstop       = uloc(p->coc, "u_fstop");
p->u_coc.sensor_w    = uloc(p->coc, "u_sensor_w");
p->u_coc.max_coc     = uloc(p->coc, "u_max_coc");
p->u_coc.texel_scale = uloc(p->coc, "u_texel_scale");
p->u_coc.near_far    = uloc(p->coc, "u_near_far");
p->u_gather.tex_color  = uloc(p->gather, "u_color");
p->u_gather.tex_coc    = uloc(p->gather, "u_coc");
p->u_gather.taps       = uloc(p->gather, "u_taps");
p->u_gather.tap_count  = uloc(p->gather, "u_tap_count");
p->u_gather.texel_size = uloc(p->gather, "u_texel_size");
p->u_gather.near_pass  = uloc(p->gather, "u_near_pass");
p->u_comp.tex_color   = uloc(p->composite, "u_color");
p->u_comp.tex_near    = uloc(p->composite, "u_near");
p->u_comp.tex_far     = uloc(p->composite, "u_far");
p->u_comp.strength    = uloc(p->composite, "u_strength");
p->u_comp.near_dilate = uloc(p->composite, "u_near_dilate");
p->u_comp.debug_view  = uloc(p->composite, "u_debug_view");
return p->ok;
float packed[DOFX_KERNEL_MAX * 2];
int floats = dof_kernel_pack(k, packed);
glUseProgram(p->gather);
glUniform2fv(p->u_gather.taps, floats / 2, packed);
if (p->u_gather.tap_count >= 0)
        glUniform1i(p->u_gather.tap_count, k->count);
glUseProgram(0);
}
