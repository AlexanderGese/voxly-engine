#ifndef SOUND_AUDIO3D_TYPES_H
#define SOUND_AUDIO3D_TYPES_H
#include <stdint.h>
#include "../../math/vec3.h"
#include "audio3d_config.h"
typedef struct {
    int16_t *samples;     // owned, length = frames
    uint32_t frames;      // sample count (mono frames)
    uint32_t rate;        // source sample rate, resampled at play time
    uint8_t  in_use;      // slot occupied?
} audio3d_clip;
typedef struct {
    vec3  pos;
    vec3  vel;
    vec3  right;          // +x of the head
    vec3  up;             // +y
    vec3  fwd;            // -z look dir, normalised
    float gain;           // master gain, 0..1
} audio3d_listener;
typedef enum {
    AUDIO3D_VOICE_FREE = 0,
    AUDIO3D_VOICE_PLAYING,
    AUDIO3D_VOICE_STOPPING   // fading out, will free when the ramp hits zero
} audio3d_voice_state;
typedef struct {
    audio3d_voice_state state;
    int      clip_id;     // index into the bank
    uint32_t handle;      // generation-tagged id handed back to callers

    vec3     pos;
    vec3     vel;

    double   cursor;      // fractional read position into the clip, in frames
    double   step_base;   // clip_rate/out_rate, the static resample ratio
    double   step;        // live increment = step_base * pitch * doppler

    float    gain;        // per-voice user gain
    float    pitch;       // user pitch multiplier
    int      looping;

    // optional directional cone. dir is the facing axis; if dir is ~zero the
    // source is omnidirectional and the cone is skipped entirely.
    vec3     cone_dir;
    float    cone_inner;  // cos of half the inner angle (full gain inside)
    float    cone_outer;  // cos of half the outer angle (outer_gain outside)
    float    cone_outer_gain;

    // smoothed mix params so we dont zipper when the source moves fast.
    float    cur_gain_l, cur_gain_r;
    float    tgt_gain_l, tgt_gain_r;
    float    cur_cutoff, tgt_cutoff;   // occlusion lowpass cutoff, hz

    // one-pole lowpass state (per output channel) for occlusion.
    float    lp_l, lp_r;

    float    fade;        // 0..1 envelope used by STOPPING and voice steals
} audio3d_voice;
#define AUDIO3D_HANDLE_NONE   0u
#define AUDIO3D_HANDLE_SLOT(h)  ((h) & 0xFFFFu)
#define AUDIO3D_HANDLE_GEN(h)   ((h) >> 16)
#define AUDIO3D_MAKE_HANDLE(slot, gen)  (((uint32_t)(gen) << 16) | ((slot) & 0xFFFFu))
#endif
