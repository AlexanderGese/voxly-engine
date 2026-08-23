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
audio3d_listener_set_gain(&s->listener, gain);
int id = audio3d_bank_find(&s->bank, name);
if (id < 0) return AUDIO3D_HANDLE_NONE;
return audio3d_pool_play(&s->pool, id, pos, gain, 1.0f, 1, &s->bank);
audio3d_pool_stop(&s->pool, h, 0);
for (int i = 0;
i < AUDIO3D_MAX_VOICES;
return audio3d_mixer_render(&s->mix, &s->pool, &s->bank,
                                &s->listener, out, frames);
}
