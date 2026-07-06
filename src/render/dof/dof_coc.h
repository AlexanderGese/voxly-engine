#ifndef RENDER_DOF_COC_H
#define RENDER_DOF_COC_H

#include "dof_config.h"

// circle of confusion. the heart of physically-ish dof: given how far a
// fragment is from the camera and where the lens is focused, how big a blur
// disk does that fragment smear into. comes straight out of the thin-lens
// equation. we keep a cpu reference here so the shader has something to be
// checked against.
//
// sign convention: a *negative* signed coc means the point is in front of the
// focal plane (near field), positive means behind it (far field). the gather
// pass cares about the magnitude; the composite cares about the sign because
// near and far blur composite differently (near bleeds over sharp, far
// doesnt).

typedef struct {
    float focus_dist;   // distance to the plane of focus (world units)
    float focal_len;    // lens focal length (world units)
    float fstop;        // aperture f-number, smaller = wider = blurrier
    float sensor_w;     // sensor width, sets the texel scaling
    float max_coc;      // clamp, in the same units as the raw lens coc
} dof_lens;

// fill a lens with the config defaults. focus_dist is the one thing you
// actually drive per frame; everything else you set once.
void  dof_lens_defaults(dof_lens *l);

// aperture diameter = focal_len / fstop. handy for the gather to know the
// physical pupil size.
float dof_lens_aperture(const dof_lens *l);

// raw thin-lens coc diameter on the sensor for a point at view-space distance
// `dist`. units are sensor units; multiply by the texel scale to get pixels.
// returns 0 at the focal plane, grows either side.
float dof_coc_lens(const dof_lens *l, float dist);

// signed coc in *texels*. negative = near field, positive = far. clamped to
// +/- max_texels. this is what both the gather mask and the composite use.
// `dist` is positive view-space depth (distance in front of the camera).
float dof_coc_signed(const dof_lens *l, float dist, float max_texels);

// is this distance effectively in focus? coc magnitude below the epsilon.
int   dof_coc_in_focus(const dof_lens *l, float dist, float epsilon);

// the inverse-ish: given a target coc in texels, what view distance behind
// the focal plane produces it. used by the autofocus/debug to reason about
// the far falloff. returns focus_dist if no real solution.
float dof_coc_far_distance(const dof_lens *l, float coc_texels, float texel_scale);

#endif
