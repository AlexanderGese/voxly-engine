#include "effects_color.h"
#include "effects_def.h"

#include <stddef.h>

bool effects_color_visible(const effects_set *s) {
    if (!s) return false;
    // invisibility suppresses the swirl entirely — kind of the point.
    if (effects_set_has(s, EFFECT_INVISIBILITY)) return false;
    for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
        const effects_instance *e = &s->slots[i];
        if (!e->active || !e->show_particles) continue;
        if (effects_def_get(e->kind)->hidden) continue;
        return true;
    }
    return false;
}

vec3 effects_color_blend(const effects_set *s) {
    if (!effects_color_visible(s)) return VEC3_ZERO;

    vec3  accum = VEC3_ZERO;
    float wsum = 0.0f;

    for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
        const effects_instance *e = &s->slots[i];
        if (!e->active || !e->show_particles) continue;
        const effects_def *d = effects_def_get(e->kind);
        if (d->hidden) continue;

        // stronger effects pull the blend toward their colour. ambient haze
        // counts for less so a beacon doesn't drown out a real potion.
        float w = (float)(e->amplifier + 1);
        if (e->ambient) w *= 0.35f;

        vec3 c = vec3_new(d->r / 255.0f, d->g / 255.0f, d->b / 255.0f);
        accum = vec3_add(accum, vec3_scale(c, w));
        wsum += w;
    }

    if (wsum <= 0.0f) return VEC3_ZERO;
    return vec3_scale(accum, 1.0f / wsum);
}

int effects_color_particle_count(const effects_set *s) {
    if (!effects_color_visible(s)) return 0;

    int strongest = 0;
    int visible = 0;
    bool all_ambient = true;

    for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
        const effects_instance *e = &s->slots[i];
        if (!e->active || !e->show_particles) continue;
        if (effects_def_get(e->kind)->hidden) continue;
        visible++;
        if (e->amplifier + 1 > strongest) strongest = e->amplifier + 1;
        if (!e->ambient) all_ambient = false;
    }
    if (visible == 0) return 0;

    // a couple per visible effect, plus a kicker for the loudest one. ambient
    // sets emit a thin trickle no matter how strong.
    int n = visible * 2 + strongest;
    if (all_ambient) n = (n + 3) / 4;
    return n;
}

uint32_t effects_color_pack(vec3 c) {
    int r = (int)(c.x * 255.0f + 0.5f);
    int g = (int)(c.y * 255.0f + 0.5f);
    int b = (int)(c.z * 255.0f + 0.5f);
    if (r < 0) r = 0;
    if (r > 255) r = 255;
    if (g < 0) g = 0;
    if (g > 255) g = 255;
    if (b < 0) b = 0;
    if (b > 255) b = 255;
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}
