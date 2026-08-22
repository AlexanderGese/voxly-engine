#include "audio3d_mixer.h"
#include "audio3d_pan.h"
#include "audio3d_dsp.h"
#include "audio3d_clip.h"
#include "audio3d_cone.h"
#include <string.h>
#include <math.h>
void audio3d_mixer_cfg_default(audio3d_mixer_cfg *cfg) {
    if (!cfg) return;
    cfg->atten     = AUDIO3D_ATTEN_INVERSE;
    cfg->ref_dist  = AUDIO3D_REF_DIST;
    cfg->max_dist  = AUDIO3D_MAX_DIST;
    cfg->rolloff   = AUDIO3D_ROLLOFF;
    cfg->pan_width = AUDIO3D_PAN_WIDTH;
    cfg->doppler   = 1;
    // ~5ms time constant. alpha = 1 - exp(-1/(tc*fs)). precomputed-ish.
    float tc = 0.005f;
    cfg->smooth = 1.0f - expf(-1.0f / (tc * (float)AUDIO3D_SAMPLE_RATE));
}

void audio3d_mixer_update_voice(const audio3d_mixer_cfg *cfg,
                                const audio3d_listener *l,
                                audio3d_voice *v, float occlusion) {
    if (!cfg || !l || !v) return;
if (occlusion < 0.0f) occlusion = 0.0f;
if (occlusion > 1.0f) occlusion = 1.0f;
float dist = vec3_distance(l->pos, v->pos);
float atten = audio3d_atten_gain(cfg->atten, dist,
                                     cfg->ref_dist, cfg->max_dist, cfg->rolloff);
float base = v->gain * atten * l->gain * AUDIO3D_VOICE_HEADROOM;
// occlusion knocks down the high end and a little of the overall level.
base *= (1.0f - 0.4f * occlusion);
// directional sources lose gain off-axis. omni voices return 1.0 here.
base *= audio3d_cone_gain(v, l->pos);
audio3d_pan pan = audio3d_pan_spatial(l, v->pos, base, cfg->pan_width);
v->tgt_gain_l = pan.left;
v->tgt_gain_r = pan.right;
// cutoff slides from nyquist (clear) down toward the occlusion cutoff.
float nyq = (float)AUDIO3D_SAMPLE_RATE * 0.5f;
v->tgt_cutoff = nyq + (AUDIO3D_OCCLUSION_CUTOFF_HZ - nyq) * occlusion;
// recompute the live resample step from the static base ratio so doppler
// never accumulates drift. step = base * pitch * doppler.
float shift = 1.0f;
if (cfg->doppler)
        shift = audio3d_doppler_shift(l->pos, l->vel, v->pos, v->vel);
v->step = v->step_base * (double)v->pitch * (double)shift;
}

// per-sample core for one voice. returns 1 while audio remains.
static int mix_voice_block(const audio3d_mixer_cfg *cfg, audio3d_voice *v,
                           const audio3d_clip *c, float *accum_l, float *accum_r,
                           uint32_t frames) {
    float fs = (float)AUDIO3D_SAMPLE_RATE;
    int alive = 1;
    for (uint32_t i = 0; i < frames && alive; i++) {
        // ramp the smoothed params toward target (zipper-free).
        v->cur_gain_l += cfg->smooth * (v->tgt_gain_l - v->cur_gain_l);
        v->cur_gain_r += cfg->smooth * (v->tgt_gain_r - v->cur_gain_r);
        v->cur_cutoff += cfg->smooth * (v->tgt_cutoff - v->cur_cutoff);

        // stopping voices ramp their envelope to zero over a few ms.
        if (v->state == AUDIO3D_VOICE_STOPPING) {
            v->fade -= cfg->smooth * 4.0f;
            if (v->fade < 0.0f) v->fade = 0.0f;
        }

        float s;
        alive = audio3d_clip_sample(c, v->cursor, v->looping, &s);

        float alpha = audio3d_dsp_lp_coeff(v->cur_cutoff, fs);
        float l = audio3d_dsp_lp_step(s, alpha, &v->lp_l) * v->cur_gain_l * v->fade;
        float r = audio3d_dsp_lp_step(s, alpha, &v->lp_r) * v->cur_gain_r * v->fade;

        accum_l[i] += l;
        accum_r[i] += r;

        v->cursor += v->step;   // step already folds in pitch + doppler
    }
    return alive;
}

int audio3d_mixer_render(const audio3d_mixer_cfg *cfg,
                         audio3d_pool *pool, const audio3d_bank *bank,
                         const audio3d_listener *l,
                         int16_t *out, uint32_t frames) {
    if (!cfg || !pool || !bank || !out || frames == 0) return 0;
// scratch accumulators on the stack. block sizes are small (a few hundred
// frames) so this is fine; if we ever want huge blocks, heap it.
enum { MAXBLK = 2048 }
;
float al[MAXBLK];
float ar[MAXBLK];
if (frames > MAXBLK) frames = MAXBLK;
// caller should chunk; clamp anyway
memset(al, 0, frames * sizeof(float));
memset(ar, 0, frames * sizeof(float));
int contributing = 0;
for (int i = 0;
i < AUDIO3D_MAX_VOICES;
i++) {
        audio3d_voice *v = &pool->voices[i];
        if (v->state == AUDIO3D_VOICE_FREE) continue;

        const audio3d_clip *c = audio3d_bank_clip(bank, v->clip_id);
        if (!c) { v->state = AUDIO3D_VOICE_STOPPING; v->fade = 0.0f; continue; }

        // skip voices that are effectively silent and not ramping up — saves
        // the resample work on far-off sources.
        float tgt = v->tgt_gain_l + v->tgt_gain_r;
        float cur = v->cur_gain_l + v->cur_gain_r;
        if (tgt < 1e-4f && cur < 1e-4f && v->state != AUDIO3D_VOICE_STOPPING) {
            // still advance the cursor so loops stay in phase with the world.
            v->cursor += v->step * (double)frames;
            continue;
        }

        int alive = mix_voice_block(cfg, v, c, al, ar, frames);
        contributing++;

        if (!alive && !v->looping) {
            // one-shot finished. let the reaper free it.
            v->state = AUDIO3D_VOICE_STOPPING;
            v->fade  = 0.0f;
        }
    }

    // sum bus -> soft clip -> s16, interleaved.
    for (uint32_t i = 0;
i < frames;
i++) {
        out[i * 2 + 0] = audio3d_dsp_to_s16(audio3d_dsp_softclip(al[i]));
        out[i * 2 + 1] = audio3d_dsp_to_s16(audio3d_dsp_softclip(ar[i]));
    }

    audio3d_pool_reap(pool);
return contributing;
}
