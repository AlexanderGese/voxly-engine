#ifndef SOUND_AUDIO3D_MIXER_H
#define SOUND_AUDIO3D_MIXER_H
#include "audio3d_types.h"
#include "audio3d_source.h"
#include "audio3d_bank.h"
#include "audio3d_listener.h"
#include "audio3d_attenuation.h"
// the mixer. given the voice pool, the bank and the listener, it renders one
// block of interleaved stereo s16. this is the hot loop of the whole thing.
typedef struct {
    audio3d_atten_model atten;       // distance model for all voices
    float    ref_dist;
    float    max_dist;
    float    rolloff;
    float    pan_width;
    int      doppler;                // enable/disable the pitch shift
    // smoothing rate for the per-voice gain/cutoff ramps, per sample. derived
    // from a time constant so it's framerate-independent.
    float    smooth;
} audio3d_mixer_cfg;
void audio3d_mixer_cfg_default(audio3d_mixer_cfg *cfg);
// resolve a voice's spatial mix params (target gains + occlusion cutoff) from
// the listener. set occlusion 0..1 (0 = clear line of sight, 1 = fully muffled)
// from whatever the world tells you; the mixer just trusts it.
void audio3d_mixer_update_voice(const audio3d_mixer_cfg *cfg,
                                const audio3d_listener *l,
                                audio3d_voice *v, float occlusion);
int  audio3d_mixer_render(const audio3d_mixer_cfg *cfg,
                          audio3d_pool *pool, const audio3d_bank *bank,
                          const audio3d_listener *l,
                          int16_t *out, uint32_t frames);
#endif
