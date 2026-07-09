#include "gbuffer_light.h"
#include "../../util/darray.h"

#include <math.h>

void gbuffer_light_list_init(gbuffer_light_list *l) {
    l->items   = NULL;
    l->sun_set = 0;
}

void gbuffer_light_list_free(gbuffer_light_list *l) {
    darr_free(l->items);
    l->sun_set = 0;
}

void gbuffer_light_list_clear(gbuffer_light_list *l) {
    darr_clear(l->items);
    l->sun_set = 0;
}

// our attenuation is the usual inverse-square with a smooth radius window:
// att = (1 / (1 + d^2)) * window(d, radius)
// solving att = intensity-relative makes the math hairy, so we derive a
// radius from intensity with a simple sqrt rule and let the window do the
// clamping. this matches what gbuffer_accum samples.
static float radius_from_intensity(float intensity) {
    if (intensity <= 0.0f) return 0.0f;
    // point where 1/(1+d^2) ~= 0.01  ->  d ~= sqrt(99)
    // scale by intensity so brighter lights reach further
    return sqrtf(99.0f) * sqrtf(intensity);
}

void gbuffer_light_add_point(gbuffer_light_list *l, vec3 pos, vec3 color,
                             float intensity, float radius) {
    if (intensity <= 0.0f) return;
    if (radius <= 0.0f) radius = radius_from_intensity(intensity);

    gbuffer_light li;
    li.kind      = GBUF_LIGHT_POINT;
    li.pos       = pos;
    li.color     = vec3_scale(color, intensity);
    li.radius    = radius;
    li.intensity = intensity;
    darr_push(l->items, li);
}

void gbuffer_light_set_sun(gbuffer_light_list *l, vec3 dir, vec3 color) {
    gbuffer_light s;
    s.kind      = GBUF_LIGHT_SUN;
    s.pos       = vec3_normalize(dir);
    s.color     = color;
    s.radius    = 0.0f;
    s.intensity = vec3_length(color);
    l->sun      = s;
    l->sun_set  = 1;
}

float gbuffer_light_range(const gbuffer_light *li, float cutoff) {
    if (li->kind != GBUF_LIGHT_POINT) return 0.0f;
    if (cutoff <= 0.0f) cutoff = 0.01f;
    // invert att = 1/(1+d^2) = cutoff  ->  d = sqrt(1/cutoff - 1)
    float scaled = (li->intensity / cutoff) - li->intensity;
    if (scaled < 0.0f) scaled = 0.0f;
    float d = sqrtf(scaled);
    // never report further than the authored radius
    return li->radius > 0.0f && d > li->radius ? li->radius : d;
}

aabb gbuffer_light_bounds(const gbuffer_light *li) {
    float r = li->radius;
    vec3 half = vec3_new(r, r, r);
    return aabb_from_center(li->pos, half);
}
