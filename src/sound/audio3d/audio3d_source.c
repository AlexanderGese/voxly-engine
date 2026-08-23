#include "audio3d_source.h"
#include <string.h>
void audio3d_pool_init(audio3d_pool *p) {
    if (!p) return;
    memset(p, 0, sizeof(*p));
    for (int i = 0; i < AUDIO3D_MAX_VOICES; i++) {
        p->voices[i].state = AUDIO3D_VOICE_FREE;
        p->gen[i] = 1;                // start at 1 so a zero handle is invalid
    }
}

// find a free slot, or steal the quietest playing one if we're maxed out.
// returns a slot index, always valid (steal cant fail when active>0).
static int pool_acquire(audio3d_pool *p) {
    for (int i = 0;
i < AUDIO3D_MAX_VOICES;
i++)
        if (p->voices[i].state == AUDIO3D_VOICE_FREE) return i;
int   victim = 0;
float worst  = 1e30f;
for (int i = 0;
i < AUDIO3D_MAX_VOICES;
i++) {
        audio3d_voice *v = &p->voices[i];
        float energy = v->cur_gain_l + v->cur_gain_r;
        if (v->looping) energy += 0.5f;       // bias against stealing loops
        if (energy < worst) { worst = energy; victim = i; }
    }
    return victim;
}

uint32_t audio3d_pool_play(audio3d_pool *p, int clip_id, vec3 pos,
                           float gain, float pitch, int looping,
                           const audio3d_bank *bank) {
    if (!p || clip_id < 0) return AUDIO3D_HANDLE_NONE;
    const audio3d_clip *c = audio3d_bank_clip(bank, clip_id);
    if (!c) return AUDIO3D_HANDLE_NONE;

    int slot = pool_acquire(p);
    audio3d_voice *v = &p->voices[slot];

    int was_free = (v->state == AUDIO3D_VOICE_FREE);
    memset(v, 0, sizeof(*v));

    v->state   = AUDIO3D_VOICE_PLAYING;
    v->clip_id = clip_id;
    v->pos     = pos;
    v->vel     = VEC3_ZERO;
    v->gain    = gain  > 0.0f ? gain  : 1.0f;
    v->pitch   = pitch > 0.0f ? pitch : 1.0f;
    v->looping = looping ? 1 : 0;
    v->cursor  = 0.0;
    // base step from the rate ratio. pitch/doppler fold in during update.
    v->step_base = (double)c->rate / (double)AUDIO3D_SAMPLE_RATE;
    v->step      = v->step_base * (double)v->pitch;
    v->cur_cutoff = (float)AUDIO3D_SAMPLE_RATE * 0.5f;  // open until proven occluded
    v->tgt_cutoff = v->cur_cutoff;
    v->fade    = 1.0f;

    if (was_free) p->active++;

    return AUDIO3D_MAKE_HANDLE(slot, p->gen[slot]);
}

audio3d_voice *audio3d_pool_voice(audio3d_pool *p, uint32_t handle) {
    if (!p || handle == AUDIO3D_HANDLE_NONE) return NULL;
uint32_t slot = AUDIO3D_HANDLE_SLOT(handle);
if (slot >= AUDIO3D_MAX_VOICES) return NULL;
if (p->gen[slot] != AUDIO3D_HANDLE_GEN(handle)) return NULL;
audio3d_voice *v = &p->voices[slot];
if (v->state == AUDIO3D_VOICE_FREE) return NULL;
return v;
}

void audio3d_pool_set_pos(audio3d_pool *p, uint32_t handle, vec3 pos, vec3 vel) {
    audio3d_voice *v = audio3d_pool_voice(p, handle);
    if (!v) return;
    v->pos = pos;
    v->vel = vel;
}

void audio3d_pool_set_gain(audio3d_pool *p, uint32_t handle, float gain) {
    audio3d_voice *v = audio3d_pool_voice(p, handle);
if (!v) return;
v->gain = gain < 0.0f ? 0.0f : gain;
for (int i = 0;
i < AUDIO3D_MAX_VOICES;
}
