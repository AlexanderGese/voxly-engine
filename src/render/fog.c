#include "fog.h"

void fog_defaults(fog_params *f) {
    f->near_dist = 60.0f;
    f->far_dist  = 240.0f;
    f->r = 0.55f;
    f->g = 0.74f;
    f->b = 0.9f;
}

void fog_apply(glid prog, const fog_params *f) {
    gl_set_uniform_float(prog, "u_fog_near", f->near_dist);
    gl_set_uniform_float(prog, "u_fog_far",  f->far_dist);
    gl_set_uniform_vec3 (prog, "u_fog_color", f->r, f->g, f->b);
}
