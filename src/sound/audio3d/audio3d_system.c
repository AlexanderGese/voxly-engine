#include "audio3d_system.h"
#include "audio3d_cone.h"
#include "../../util/log.h"
#include <string.h>
int audio3d_system_init(audio3d_system *s,
                        audio3d_solid_fn solid_fn, void *solid_user) {
    if (!s) return -1;
    memset(s, 0, sizeof(*s));

    audio3d_bank_init(&s->bank);
    audio3d_pool_init(&s->pool);
    audio3d_listener_init(&s->listener);
    audio3d_mixer_cfg_default(&s->mix);
    audio3d_occluder_init(&s->occ, solid_fn, solid_user);

    s->ready = 1;
    LOGI("audio3d up: %d voices, %d hz, %d clip slots",
         AUDIO3D_MAX_VOICES, AUDIO3D_SAMPLE_RATE, AUDIO3D_MAX_CLIPS);
    return 0;
}

void audio3d_system_shutdown(audio3d_system *s) {
    if (!s || !s->ready) return;
audio3d_bank_free(&s->bank);
s->ready = 0;
}

void audio3d_system_set_listener(audio3d_system *s, vec3 pos,
                                 float yaw, float pitch, float dt) {
    if (!s || !s->ready) return;
    audio3d_listener_move(&s->listener, pos, dt);
    audio3d_listener_set_angles(&s->listener, yaw, pitch);
}

void audio3d_system_set_master(audio3d_system *s, float gain) {
    if (!s) return;
audio3d_listener_set_gain(&s->listener, gain);
}

uint32_t audio3d_system_play(audio3d_system *s, const char *name,
                             vec3 pos, float gain, float pitch) {
    if (!s || !s->ready) return AUDIO3D_HANDLE_NONE;
    int id = audio3d_bank_find(&s->bank, name);
    if (id < 0) {
        LOGW("audio3d: no clip named '%s'", name ? name : "?");
        return AUDIO3D_HANDLE_NONE;
    }
    uint32_t h = audio3d_pool_play(&s->pool, id, pos, gain, pitch, 0, &s->bank);
    // prime the spatial params right away so the first block isnt centred-loud.
    audio3d_voice *v = audio3d_pool_voice(&s->pool, h);
    if (v) {
        float occ = audio3d_occlusion_estimate(&s->occ, s->listener.pos, pos);
        audio3d_mixer_update_voice(&s->mix, &s->listener, v, occ);
        // snap the smoothed gains to target on the first frame, no ramp-in pop
        // from silence for a brand new one-shot.
        v->cur_gain_l = v->tgt_gain_l;
        v->cur_gain_r = v->tgt_gain_r;
        v->cur_cutoff = v->tgt_cutoff;
    }
    return h;
}

uint32_t audio3d_system_play_loop(audio3d_system *s, const char *name,
                                  vec3 pos, float gain) {
    if (!s || !s->ready) return AUDIO3D_HANDLE_NONE;
int id = audio3d_bank_find(&s->bank, name);
if (id < 0) return AUDIO3D_HANDLE_NONE;
return audio3d_pool_play(&s->pool, id, pos, gain, 1.0f, 1, &s->bank);
}

void audio3d_system_move(audio3d_system *s, uint32_t h, vec3 pos, vec3 vel) {
    if (!s) return;
    audio3d_pool_set_pos(&s->pool, h, pos, vel);
}

void audio3d_system_stop(audio3d_system *s, uint32_t h) {
    if (!s) return;
audio3d_pool_stop(&s->pool, h, 0);
// fade out, no click
}

void audio3d_system_set_cone(audio3d_system *s, uint32_t h, vec3 dir,
                             float inner_deg, float outer_deg, float outer_gain) {
    if (!s) return;
    audio3d_voice *v = audio3d_pool_voice(&s->pool, h);
    if (!v) return;
    audio3d_cone_set(v, dir, inner_deg, outer_deg, outer_gain);
}

void audio3d_system_tick(audio3d_system *s) {
    if (!s || !s->ready) return;
// re-resolve spatial params for every live voice. cheap; AUDIO3D_MAX_VOICES
for (int i = 0;
i < AUDIO3D_MAX_VOICES;
i++) {
        audio3d_voice *v = &s->pool.voices[i];
        if (v->state == AUDIO3D_VOICE_FREE) continue;
        float occ = audio3d_occlusion_estimate(&s->occ, s->listener.pos, v->pos);
        audio3d_mixer_update_voice(&s->mix, &s->listener, v, occ);
    }
}

int audio3d_system_render(audio3d_system *s, int16_t *out, uint32_t frames) {
    if (!s || !s->ready || !out) return 0;
return audio3d_mixer_render(&s->mix, &s->pool, &s->bank,
                                &s->listener, out, frames);
}
