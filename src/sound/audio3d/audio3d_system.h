#ifndef SOUND_AUDIO3D_SYSTEM_H
#define SOUND_AUDIO3D_SYSTEM_H
#include "audio3d_types.h"
#include "audio3d_bank.h"
#include "audio3d_source.h"
#include "audio3d_mixer.h"
#include "audio3d_occlusion.h"
// the front door. owns the bank, the voice pool, the listener and the mixer
// config, and drives the per-frame update + per-block render. the old stub
// sound.c can forward into this once someone wires an audio device callback.
typedef struct {
    audio3d_bank      bank;
    audio3d_pool      pool;
    audio3d_listener  listener;
    audio3d_mixer_cfg mix;
    audio3d_occluder  occ;
    int               ready;
} audio3d_system;
// bring the whole thing up. wires sane defaults. solid_fn/user may be NULL if
// you dont have a world yet (sources just wont be occluded).
int  audio3d_system_init(audio3d_system *s,
                         audio3d_solid_fn solid_fn, void *solid_user);
void audio3d_system_shutdown(audio3d_system *s);
// update the listener from the camera. call once per game tick. dt drives the
// listener velocity used for doppler.
void audio3d_system_set_listener(audio3d_system *s, vec3 pos,
                                 float yaw, float pitch, float dt);
void audio3d_system_set_master(audio3d_system *s, float gain);
// fire a positional one-shot. returns a handle you can stop/move, or NONE.
uint32_t audio3d_system_play(audio3d_system *s, const char *name,
                             vec3 pos, float gain, float pitch);
uint32_t audio3d_system_play_loop(audio3d_system *s, const char *name,
                                  vec3 pos, float gain);
void audio3d_system_move(audio3d_system *s, uint32_t h, vec3 pos, vec3 vel);
void audio3d_system_stop(audio3d_system *s, uint32_t h);
// make a playing voice directional. angles in degrees (full cone). no-op on a
// dead handle. pass inner_deg >= 360 to go back to omnidirectional.
void audio3d_system_set_cone(audio3d_system *s, uint32_t h, vec3 dir,
                             float inner_deg, float outer_deg, float outer_gain);
// recompute every live voice's spatial params against the current listener.
// run this once per game tick, before the audio callback renders blocks.
void audio3d_system_tick(audio3d_system *s);
// render one block of interleaved stereo s16 — call this from the device
// callback. returns voices that contributed.
int  audio3d_system_render(audio3d_system *s, int16_t *out, uint32_t frames);
#endif
