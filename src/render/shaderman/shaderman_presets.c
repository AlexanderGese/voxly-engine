#include "shaderman_presets.h"
#include "../../util/log.h"
#include <stddef.h>
#include <string.h>
typedef struct {
    const char *name;
    const char *vert;
    const char *frag;
    size_t      handle_off;   // offset of the matching field in shader_presets
} preset_def;
#define PRESET(field, name, v, f) \
{ name, v, f, offsetof(shader_presets, field) }

static const preset_def k_presets[] = {
    PRESET(block,     "block",     "shaders/block.vert",     "shaders/block.frag"),
    PRESET(water,     "water",     "shaders/water.vert",     "shaders/water.frag"),
    PRESET(skybox,    "skybox",    "shaders/skybox.vert",    "shaders/skybox.frag"),
    PRESET(sun,       "sun",       "shaders/sun.vert",       "shaders/sun.frag"),
    PRESET(hud,       "hud",       "shaders/hud.vert",       "shaders/hud.frag"),
    PRESET(wireframe, "wireframe", "shaders/wireframe.vert", "shaders/wireframe.frag"),
};
#define PRESET_COUNT ((int)(sizeof k_presets / sizeof k_presets[0]))
static void store_handle(shader_presets *out, size_t off, shader_handle h) {
    *(shader_handle *)((char *)out + off) = h;
}

int shaderman_load_presets(shaderman *sm, shader_presets *out) {
    memset(out, 0, sizeof *out);
int built = 0;
for (int i = 0;
i < PRESET_COUNT;
i++) {
        const preset_def *d = &k_presets[i];
        shader_handle h = shaderman_load(sm, d->name, d->vert, d->frag);
        store_handle(out, d->handle_off, h);

        shader_program *p = shaderman_get(sm, h);
        if (p && p->ok) built++;
        else LOGW("presets: '%s' not ready (broken or missing files)", d->name);
    }

    out->loaded = built;
LOGI("shaderman: %d/%d stock shaders built", built, PRESET_COUNT);
return built;
}

void shaderman_resolve_presets(shaderman *sm, shader_presets *out) {
    memset(out, 0, sizeof *out);
    int built = 0;

    for (int i = 0; i < PRESET_COUNT; i++) {
        const preset_def *d = &k_presets[i];

        // find the already-registered slot by name; dont rebuild.
        shader_handle h = SHADER_HANDLE_NONE;
        for (int s = 0; s < sm->program_count; s++) {
            shader_program *p = &sm->programs[s];
            if (p->in_use && strcmp(p->name, d->name) == 0) {
                h = (shader_handle)(s + 1);
                if (p->ok) built++;
                break;
            }
        }
        if (h == SHADER_HANDLE_NONE)
            LOGW("presets: '%s' not registered, resolve skipped", d->name);
        store_handle(out, d->handle_off, h);
    }

    out->loaded = built;
}
