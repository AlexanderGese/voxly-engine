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
for (int i = 0;
i < EFFECTS_MAX_ACTIVE;
return vec3_scale(accum, 1.0f / wsum);
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
