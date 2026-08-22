#ifndef SOUND_AUDIO3D_ATTENUATION_H
#define SOUND_AUDIO3D_ATTENUATION_H

#include "audio3d_types.h"

// distance attenuation models. pick one, they all map a distance in blocks to
// a linear gain 0..1. the inverse-clamped one is what openal calls the
// default; i kept the others around because the linear one is cheaper and
// sometimes you just want a falloff you can reason about.

typedef enum {
    AUDIO3D_ATTEN_NONE = 0,    // flat gain, ignores distance
    AUDIO3D_ATTEN_LINEAR,      // lerp ref..max -> 1..0
    AUDIO3D_ATTEN_INVERSE,     // ref / (ref + rolloff*(d-ref))
    AUDIO3D_ATTEN_EXPONENTIAL  // (d/ref)^-rolloff
} audio3d_atten_model;

// gain for a given distance. ref/max/rolloff come from config but are passed
// so per-source overrides are possible later.
float audio3d_atten_gain(audio3d_atten_model model,
                         float dist, float ref, float max, float rolloff);

// doppler pitch multiplier. source_vel/listener_vel projected onto the line
// between them. clamped so a fast flyby cant invert the frequency.
float audio3d_doppler_shift(vec3 listener_pos, vec3 listener_vel,
                            vec3 source_pos, vec3 source_vel);

#endif
