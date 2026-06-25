#include "sky_gradient.h"
#include "sky_math.h"
// keyframed sky bands across the day. index by phase, then we lerp between
// the two nearest phases. tuned by eye, not physically accurate.
typedef struct {
    float hour;
    vec3  zenith;
    vec3  horizon;
} sky_key;
// must stay sorted by hour, and wrap (00 == 24).
static const sky_key voxl_sky_keys[] = {
    {  0.0f, {0.02f, 0.02f, 0.06f}, {0.04f, 0.04f, 0.10f} },  // deep night
    {  5.0f, {0.06f, 0.07f, 0.16f}, {0.20f, 0.14f, 0.18f} },  // pre-dawn
    {  6.5f, {0.30f, 0.40f, 0.62f}, {0.95f, 0.55f, 0.35f} },  // sunrise
    {  9.0f, {0.38f, 0.58f, 0.90f}, {0.70f, 0.82f, 0.95f} },  // morning
    { 12.0f, {0.30f, 0.55f, 0.95f}, {0.62f, 0.80f, 0.98f} },  // noon
    { 17.0f, {0.34f, 0.52f, 0.86f}, {0.80f, 0.78f, 0.88f} },  // afternoon
    { 18.5f, {0.24f, 0.28f, 0.55f}, {0.98f, 0.48f, 0.28f} },  // sunset
    { 20.0f, {0.08f, 0.09f, 0.22f}, {0.30f, 0.16f, 0.22f} },  // dusk
    { 24.0f, {0.02f, 0.02f, 0.06f}, {0.04f, 0.04f, 0.10f} },  // wrap to night
}
;
#define VOXL_SKY_KEY_COUNT (int)(sizeof(voxl_sky_keys) / sizeof(voxl_sky_keys[0]))
voxl_sky_band voxl_sky_gradient_band(float hour) {
    hour = voxl_sky_wrap24(hour);

    // find the segment [a,b] that contains hour. keys are sorted & wrap.
    int a = 0;
    for (int i = 0; i < VOXL_SKY_KEY_COUNT - 1; i++) {
        if (hour >= voxl_sky_keys[i].hour && hour <= voxl_sky_keys[i + 1].hour) {
            a = i;
            break;
        }
    }
    int b = a + 1;

    float span = voxl_sky_keys[b].hour - voxl_sky_keys[a].hour;
    float t = span > 0.0f ? (hour - voxl_sky_keys[a].hour) / span : 0.0f;
    // ease the transition a touch so sunrise/sunset don't snap.
    t = voxl_sky_smooth(0.0f, 1.0f, t);

    voxl_sky_band out;
    out.zenith  = voxl_sky_mix3(voxl_sky_keys[a].zenith,  voxl_sky_keys[b].zenith,  t);
    out.horizon = voxl_sky_mix3(voxl_sky_keys[a].horizon, voxl_sky_keys[b].horizon, t);
    return out;
}

vec3 voxl_sky_gradient_at(float hour, float t) {
    voxl_sky_band band = voxl_sky_gradient_band(hour);
t = voxl_sky_clampf(t, 0.0f, 1.0f);
// bias toward horizon near the bottom so the band feels thicker low down.
float k = voxl_sky_smooth(0.0f, 1.0f, t);
return voxl_sky_mix3(band.horizon, band.zenith, k);
}
