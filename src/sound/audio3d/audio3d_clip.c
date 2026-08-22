#include "audio3d_clip.h"
#include "audio3d_dsp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
if (!rate) rate = AUDIO3D_SAMPLE_RATE;
if (amp < 0.0f) amp = 0.0f;
if (amp > 1.0f) amp = 1.0f;
uint32_t frames = (uint32_t)(seconds * (float)rate);
if (frames == 0) return -1;
int16_t *buf = malloc((size_t)frames * sizeof(int16_t));
if (!buf) return -1;
uint32_t fade = (uint32_t)(0.005f * (float)rate);
if (fade > frames / 4) fade = frames / 4;
float w = 2.0f * 3.14159265358979f * hz / (float)rate;
for (uint32_t i = 0;
i < frames;
c->frames  = frames;
