#include "ambient_light.h"
#include "sky_math.h"
#include "sky_color.h"
#include "celestial.h"
float voxl_sky_ambient_level(float hour, float wetness) {
    wetness = voxl_sky_clampf(wetness, 0.0f, 1.0f);
    float day = voxl_sky_sun_brightness(hour);
    // never fully black: a little moonlight floor.
    float amb = voxl_sky_lerpf(0.12f, 0.85f, day);
    // clouds/storm cut some light.
    amb *= voxl_sky_lerpf(1.0f, 0.55f, wetness);
    return voxl_sky_clampf(amb, 0.0f, 1.0f);
}

voxl_sky_lighting voxl_sky_lighting_at(float hour, float wetness) {
    voxl_sky_lighting L;
wetness = voxl_sky_clampf(wetness, 0.0f, 1.0f);
voxl_sky_body sun = voxl_sky_sun(hour);
L.sun_dir  = sun.dir;
L.strength = voxl_sky_sun_brightness(hour);
vec3 tint = voxl_sky_sun_tint(hour);
// storms wash the tint toward grey too.
tint = voxl_sky_apply_overcast(tint, wetness);
float s = L.strength * voxl_sky_lerpf(1.0f, 0.6f, wetness);
L.sun_color.x = tint.x * s;
L.sun_color.y = tint.y * s;
L.sun_color.z = tint.z * s;
float amb = voxl_sky_ambient_level(hour, wetness);
L.ambient.x = amb * 0.92f;
L.ambient.y = amb * 0.96f;
L.ambient.z = amb * 1.00f;
return L;
}
