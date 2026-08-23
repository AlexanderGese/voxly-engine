#ifndef SOUND_AUDIO3D_SOURCE_H
#define SOUND_AUDIO3D_SOURCE_H

#include "audio3d_types.h"
#include "audio3d_bank.h"

// the voice pool. owns AUDIO3D_MAX_VOICES slots and hands out generation-tagged
// handles so callers can poke a playing sound (move it, stop it) without us
// worrying about them touching a recycled slot.

typedef struct {
    audio3d_voice voices[AUDIO3D_MAX_VOICES];
    uint16_t      gen[AUDIO3D_MAX_VOICES];   // bumped every time a slot recycles
    int          active;                      // count of non-free voices
} audio3d_pool;

void audio3d_pool_init(audio3d_pool *p);

// spawn a positional voice playing clip_id. returns a handle, or
// AUDIO3D_HANDLE_NONE if no slot was free and nothing could be stolen.
uint32_t audio3d_pool_play(audio3d_pool *p, int clip_id, vec3 pos,
                           float gain, float pitch, int looping,
                           const audio3d_bank *bank);

// resolve a handle to its live voice, or NULL if it's stale / freed.
audio3d_voice *audio3d_pool_voice(audio3d_pool *p, uint32_t handle);

// update a playing voice's position + velocity. no-op on a dead handle.
void audio3d_pool_set_pos(audio3d_pool *p, uint32_t handle, vec3 pos, vec3 vel);
void audio3d_pool_set_gain(audio3d_pool *p, uint32_t handle, float gain);

// request a stop. fades out over a few ms rather than clicking. immediate=1
// frees right away (use for teardown).
void audio3d_pool_stop(audio3d_pool *p, uint32_t handle, int immediate);

// free any voices whose envelope has reached zero. mixer calls this each block.
void audio3d_pool_reap(audio3d_pool *p);

#endif
