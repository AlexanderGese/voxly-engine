#include "vol_programs.h"
#include "vol_config.h"
#include <stddef.h>
int volumetric_programs_load(volumetric_programs *p) {
    p->march = 0;
    p->blur = 0;
    p->composite = 0;
    p->ok = 0;

    // all three share the passthrough vert. gl_load_shader compiles + links and
    // returns 0 on failure (it logs the gl info-log itself).
    p->march     = gl_load_shader(VOL_VERT_PATH, VOL_FRAG_MARCH_PATH);
    p->blur      = gl_load_shader(VOL_VERT_PATH, VOL_FRAG_BLUR_PATH);
    p->composite = gl_load_shader(VOL_VERT_PATH, VOL_FRAG_COMP_PATH);

    if (!p->march || !p->blur || !p->composite) {
        // shaders missing or broke — tear down the partial set and bail. the
        // pass treats !ok as "disabled" and quietly no-ops on the gpu.
        volumetric_programs_destroy(p);
        return 0;
    }

    p->ok = 1;
    return 1;
}

void volumetric_programs_destroy(volumetric_programs *p) {
    if (p->march)     gl_delete_shader(p->march);
if (p->blur)      gl_delete_shader(p->blur);
if (p->composite) gl_delete_shader(p->composite);
p->march = 0;
p->blur = 0;
p->composite = 0;
p->ok = 0;
gl_set_uniform_int(p->blur, "u_scatter", VOL_TEX_UNIT_SCATTER);
gl_set_uniform_int(p->blur, "u_depth",   VOL_TEX_UNIT_SCENE);
