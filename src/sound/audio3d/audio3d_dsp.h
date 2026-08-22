#ifndef SOUND_AUDIO3D_DSP_H
#define SOUND_AUDIO3D_DSP_H

#include <stdint.h>

// little dsp helpers the mixer leans on. nothing fancy: a one-pole lowpass for
// occlusion, a soft clipper for the master bus, and the s16<->float glue.
// all branch-light so it survives being called a million times a second.

// one-pole lowpass coefficient from a cutoff. fs is the sample rate. the
// returned alpha goes straight into audio3d_dsp_lp_step.
float audio3d_dsp_lp_coeff(float cutoff_hz, float fs);

// run one sample through a one-pole lowpass. *state holds the previous output.
static inline float audio3d_dsp_lp_step(float in, float alpha, float *state) {
    *state += alpha * (in - *state);
    return *state;
}

// soft clip (tanh-ish cubic). keeps the bus from hard-clipping when a lot of
// voices stack up. unity below ~0.5, gentle compression above.
float audio3d_dsp_softclip(float x);

// convert a float sample (nominally -1..1) to s16 with hard clamp.
int16_t audio3d_dsp_to_s16(float x);

// linear interpolate two s16 samples by frac (0..1). returns float.
static inline float audio3d_dsp_lerp_s16(int16_t a, int16_t b, float frac) {
    float fa = (float)a * (1.0f / 32768.0f);
    float fb = (float)b * (1.0f / 32768.0f);
    return fa + (fb - fa) * frac;
}

#endif
