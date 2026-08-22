#ifndef SOUND_AUDIO3D_CLIP_H
#define SOUND_AUDIO3D_CLIP_H

#include "audio3d_types.h"

// clip = a chunk of mono pcm plus its native rate. the bank owns the storage;
// these helpers just build/destroy and sample one.

// allocate a clip from raw mono s16 samples. copies the data so the caller can
// free its buffer. returns 0 on success.
int  audio3d_clip_load_pcm(audio3d_clip *c, const int16_t *pcm,
                           uint32_t frames, uint32_t rate);

// synthesize a simple test tone (sine) at hz for the given duration. handy for
// debugging the mixer before any real assets exist — i used this a lot.
int  audio3d_clip_make_tone(audio3d_clip *c, float hz, float seconds,
                            uint32_t rate, float amp);

void audio3d_clip_free(audio3d_clip *c);

// sample the clip at a fractional frame position with linear interpolation.
// handles looping vs one-shot at the edge. writes the interpolated sample to
// *out and returns 1 if there's still audio, 0 if a one-shot ran off the end.
int  audio3d_clip_sample(const audio3d_clip *c, double cursor, int looping,
                         float *out);

#endif
