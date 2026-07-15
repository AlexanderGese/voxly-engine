#include "shadow_frustum.h"
#include <math.h>
decompose_view(cam_view, &right, &up, &fwd, &eye);
float tan_v = tanf(fov_rad * 0.5f);
float nh = near_d * tan_v;
float nw = nh * aspect;
float fh = far_d * tan_v;
float fw = fh * aspect;
vec3 nc = vec3_add(eye, vec3_scale(fwd, near_d));
vec3 fc = vec3_add(eye, vec3_scale(fwd, far_d));
vec3 ru_n_w = vec3_scale(right, nw);
vec3 up_n_h = vec3_scale(up, nh);
vec3 ru_f_w = vec3_scale(right, fw);
vec3 up_f_h = vec3_scale(up, fh);
out->c[0] = vec3_sub(vec3_sub(nc, ru_n_w), up_n_h);
out->c[1] = vec3_sub(vec3_add(nc, ru_n_w), up_n_h);
out->c[2] = vec3_add(vec3_add(nc, ru_n_w), up_n_h);
out->c[3] = vec3_add(vec3_sub(nc, ru_n_w), up_n_h);
out->c[4] = vec3_sub(vec3_sub(fc, ru_f_w), up_f_h);
out->c[5] = vec3_sub(vec3_add(fc, ru_f_w), up_f_h);
out->c[6] = vec3_add(vec3_add(fc, ru_f_w), up_f_h);
