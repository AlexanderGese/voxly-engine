#include "audio3d_dsp.h"

#include <math.h>

float audio3d_dsp_lp_coeff(float cutoff_hz, float fs) {
    if (cutoff_hz <= 0.0f) return 0.0f;        // fully closed
    if (cutoff_hz >= fs * 0.5f) return 1.0f;   // wide open, no filtering

    // standard one-pole rc->alpha mapping. dt = 1/fs, rc = 1/(2*pi*fc).
    float dt = 1.0f / fs;
    float rc = 1.0f / (2.0f * 3.14159265358979f * cutoff_hz);
    float a = dt / (rc + dt);
    if (a < 0.0f) a = 0.0f;
    if (a > 1.0f) a = 1.0f;
    return a;
}

float audio3d_dsp_softclip(float x) {
    // cubic soft clip. linear up to 1/3, then curves, flat past 2/3.
    // cheaper than tanh and good enough for a games bus limiter.
    if (x <= -1.0f) return -2.0f / 3.0f;
    if (x >=  1.0f) return  2.0f / 3.0f;
    return x - (x * x * x) / 3.0f;
}

int16_t audio3d_dsp_to_s16(float x) {
    // scale and clamp. note we map full-scale to 32767/-32768.
    float s = x * 32767.0f;
    if (s >  32767.0f) s =  32767.0f;
    if (s < -32768.0f) s = -32768.0f;
    return (int16_t)lrintf(s);
}
