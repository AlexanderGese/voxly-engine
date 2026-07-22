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
float k  = 1.0f - eta * eta * (1.0f - ci * ci);
if (k < 0.0f) return 0;
float s = eta * ci - sqrtf(k);
out->x = eta * incident.x + s * normal.x;
out->y = eta * incident.y + s * normal.y;
out->z = eta * incident.z + s * normal.z;
return 1;
;
}
