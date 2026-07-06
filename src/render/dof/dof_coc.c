#include "dof_coc.h"

#include <math.h>

// thin-lens circle of confusion. the textbook relation is
//
// coc = A * |S2 - S1| / S2 * f / (S1 - f)
//
// where A is the aperture diameter, S1 the focus distance, S2 the subject
// distance and f the focal length. we fold the constant lens terms together
// and clamp the result; nobody's grading us on optical accuracy, it just has
// to look like a lens and stay monotonic on each side of focus.

void dof_lens_defaults(dof_lens *l) {
    l->focus_dist = DOFX_DEFAULT_FOCUS_DIST;
    l->focal_len  = DOFX_DEFAULT_FOCAL_LEN;
    l->fstop      = DOFX_DEFAULT_FSTOP;
    l->sensor_w   = DOFX_DEFAULT_SENSOR;
    l->max_coc    = DOFX_MAX_COC_TEXELS;
}

float dof_lens_aperture(const dof_lens *l) {
    // diameter of the entrance pupil. f-number is focal/diameter by
    // definition, so diameter is just focal/fstop. guard the divide.
    if (l->fstop <= 1e-4f) return l->focal_len; // wide open, dont nan out
    return l->focal_len / l->fstop;
}

float dof_coc_lens(const dof_lens *l, float dist) {
    // raw coc on the sensor, in sensor units (e.g. metres of glass). returns
    // 0 at the focal plane and grows on either side.
    float s1 = l->focus_dist;
    float f  = l->focal_len;

    // a subject exactly at the focal length is degenerate (image at infinity).
    // shove it just past so the divide stays finite.
    if (dist <= f) dist = f + 1e-4f;
    if (s1   <= f) s1   = f + 1e-4f;

    float aperture = dof_lens_aperture(l);

    // magnification of the focal plane image, times the relative defocus.
    float m = f / (s1 - f);
    float coc = aperture * fabsf(dist - s1) / dist * m;
    return coc < 0.0f ? 0.0f : coc;
}

float dof_coc_signed(const dof_lens *l, float dist, float max_texels) {
    // convert sensor coc into texels. the sensor maps across the framebuffer
    // width, so one sensor unit is (fb_width / sensor_w) texels. we dont know
    // fb width here so we bake a normalized scale: coc/sensor_w gives a
    // fraction of the frame, and the gather multiplies by the real width.
    float raw = dof_coc_lens(l, dist);
    float texels = (raw / l->sensor_w) * max_texels;

    if (texels > max_texels) texels = max_texels;

    // sign: in front of focus -> near field -> negative.
    return (dist < l->focus_dist) ? -texels : texels;
}

int dof_coc_in_focus(const dof_lens *l, float dist, float epsilon) {
    float c = dof_coc_signed(l, dist, l->max_coc);
    return fabsf(c) <= epsilon;
}

float dof_coc_far_distance(const dof_lens *l, float coc_texels, float texel_scale) {
    // invert the far branch: find the subject distance behind the focal plane
    // whose coc equals coc_texels. only the far side is single-valued in a way
    // worth solving, the near side asymptotes.
    if (coc_texels <= 0.0f || texel_scale <= 0.0f) return l->focus_dist;

    float aperture = dof_lens_aperture(l);
    float f  = l->focal_len;
    float s1 = l->focus_dist;
    float m  = f / (s1 - f);

    // target raw sensor coc from the requested texel count.
    float raw = (coc_texels / texel_scale) * l->sensor_w;

    // coc = aperture * (dist - s1)/dist * m  for dist > s1
    // = aperture*m*(1 - s1/dist)
    // solve for dist:
    float k = aperture * m;
    if (k <= 1e-6f) return l->focus_dist;

    float ratio = 1.0f - raw / k;
    if (ratio <= 1e-4f) return INFINITY; // hyperfocal: everything behind blurs equally
    return s1 / ratio;
}
