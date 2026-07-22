#include "water_fresnel.h"
#include "water_config.h"

#include <math.h>

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static float dot3(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float water_fresnel_from_cos(float cos_theta) {
    cos_theta = clampf(cos_theta, 0.0f, 1.0f);
    float m = 1.0f - cos_theta;
    // schlick: F0 + (1-F0) * (1-cos)^5
    float m5 = m * m;
    m5 = m5 * m5 * m;           // pow(m, 5) cheaply
    float f = WATER_F0 + (1.0f - WATER_F0) * m5;
    return clampf(f, WATER_FRESNEL_MIN, WATER_FRESNEL_MAX);
}

float water_fresnel_schlick(vec3 view_dir, vec3 normal) {
    // view_dir points from the surface toward the eye. cos = max(0, v.n)
    float c = dot3(view_dir, normal);
    return water_fresnel_from_cos(c);
}

vec3 water_reflect(vec3 incident, vec3 normal) {
    float d = 2.0f * dot3(incident, normal);
    return (vec3){
        incident.x - d * normal.x,
        incident.y - d * normal.y,
        incident.z - d * normal.z,
    };
}

int water_refract(vec3 incident, vec3 normal, float eta, vec3 *out) {
    float ci = -dot3(incident, normal);
    float k  = 1.0f - eta * eta * (1.0f - ci * ci);
    if (k < 0.0f) return 0;     // total internal reflection
    float s = eta * ci - sqrtf(k);
    out->x = eta * incident.x + s * normal.x;
    out->y = eta * incident.y + s * normal.y;
    out->z = eta * incident.z + s * normal.z;
    return 1;
}

vec3 water_depth_tint(float depth_blocks) {
    float t = clampf(depth_blocks / WATER_TINT_DEPTH, 0.0f, 1.0f);
    // ease so the transition isnt linear-looking. smoothstep.
    t = t * t * (3.0f - 2.0f * t);
    vec3 shallow = { WATER_TINT_SHALLOW_R, WATER_TINT_SHALLOW_G, WATER_TINT_SHALLOW_B };
    vec3 deep    = { WATER_TINT_DEEP_R,    WATER_TINT_DEEP_G,    WATER_TINT_DEEP_B };
    return (vec3){
        shallow.x + (deep.x - shallow.x) * t,
        shallow.y + (deep.y - shallow.y) * t,
        shallow.z + (deep.z - shallow.z) * t,
    };
}

vec3 water_mix_fresnel(vec3 refraction_rgb, vec3 reflection_rgb, float fresnel) {
    fresnel = clampf(fresnel, 0.0f, 1.0f);
    return (vec3){
        refraction_rgb.x + (reflection_rgb.x - refraction_rgb.x) * fresnel,
        refraction_rgb.y + (reflection_rgb.y - refraction_rgb.y) * fresnel,
        refraction_rgb.z + (reflection_rgb.z - refraction_rgb.z) * fresnel,
    };
}
