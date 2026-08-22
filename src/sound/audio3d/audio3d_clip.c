#include "audio3d_clip.h"
#include "audio3d_dsp.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

int audio3d_clip_load_pcm(audio3d_clip *c, const int16_t *pcm,
                          uint32_t frames, uint32_t rate) {
    if (!c || !pcm || frames == 0) return -1;
    c->samples = malloc((size_t)frames * sizeof(int16_t));
    if (!c->samples) return -1;
    memcpy(c->samples, pcm, (size_t)frames * sizeof(int16_t));
    c->frames = frames;
    c->rate   = rate ? rate : AUDIO3D_SAMPLE_RATE;
    c->in_use = 1;
    return 0;
}

int audio3d_clip_make_tone(audio3d_clip *c, float hz, float seconds,
                           uint32_t rate, float amp) {
    if (!c || hz <= 0.0f || seconds <= 0.0f) return -1;
    if (!rate) rate = AUDIO3D_SAMPLE_RATE;
    if (amp < 0.0f) amp = 0.0f;
    if (amp > 1.0f) amp = 1.0f;

    uint32_t frames = (uint32_t)(seconds * (float)rate);
    if (frames == 0) return -1;
    int16_t *buf = malloc((size_t)frames * sizeof(int16_t));
    if (!buf) return -1;

    // sine with a short raised-cosine fade in/out so a looped tone doesnt tick
    // at the seam. fade length is ~5ms or a quarter of the clip, whichever's
    // smaller.
    uint32_t fade = (uint32_t)(0.005f * (float)rate);
    if (fade > frames / 4) fade = frames / 4;

    float w = 2.0f * 3.14159265358979f * hz / (float)rate;
    for (uint32_t i = 0; i < frames; i++) {
        float s = sinf(w * (float)i) * amp;
        float env = 1.0f;
        if (fade > 0) {
            if (i < fade)
                env = 0.5f - 0.5f * cosf(3.14159265f * (float)i / (float)fade);
            else if (i >= frames - fade) {
                uint32_t k = frames - 1 - i;
                env = 0.5f - 0.5f * cosf(3.14159265f * (float)k / (float)fade);
            }
        }
        buf[i] = audio3d_dsp_to_s16(s * env);
    }

    c->samples = buf;
    c->frames  = frames;
    c->rate    = rate;
    c->in_use  = 1;
    return 0;
}

void audio3d_clip_free(audio3d_clip *c) {
    if (!c) return;
    free(c->samples);
    c->samples = NULL;
    c->frames  = 0;
    c->in_use  = 0;
}

int audio3d_clip_sample(const audio3d_clip *c, double cursor, int looping,
                        float *out) {
    if (!c || !c->samples || c->frames == 0) { if (out) *out = 0.0f; return 0; }

    double pos = cursor;
    if (looping) {
        // wrap into range. fmod can return negative for negative input but our
        // cursor is always >= 0, so this is fine.
        pos = fmod(pos, (double)c->frames);
        if (pos < 0.0) pos += (double)c->frames;
    } else if (pos >= (double)(c->frames - 1)) {
        // last valid sample, then we're done.
        if (out) *out = (float)c->samples[c->frames - 1] * (1.0f / 32768.0f);
        return 0;
    }

    uint32_t i0 = (uint32_t)pos;
    uint32_t i1 = i0 + 1;
    if (i1 >= c->frames) i1 = looping ? 0 : c->frames - 1;
    float frac = (float)(pos - (double)i0);

    if (out) *out = audio3d_dsp_lerp_s16(c->samples[i0], c->samples[i1], frac);
    return 1;
}
