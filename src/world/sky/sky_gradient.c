#include "sky_gradient.h"
#include "sky_math.h"
typedef struct {
    float hour;
    vec3  zenith;
    vec3  horizon;
} sky_key;
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
t = voxl_sky_clampf(t, 0.0f, 1.0f);
float k = voxl_sky_smooth(0.0f, 1.0f, t);
return voxl_sky_mix3(band.horizon, band.zenith, k);
}
