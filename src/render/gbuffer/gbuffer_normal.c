#include "gbuffer_normal.h"
#include <math.h>
static float sign_nz(float v) {
    // returns +1 for >=0, -1 otherwise. zero counts as positive on purpose,
    // matches the reference octahedral impl.
    return v >= 0.0f ? 1.0f : -1.0f;
}

gbuffer_oct gbuffer_normal_encode(vec3 n) {
    float inv_l1 = fabsf(n.x) + fabsf(n.y) + fabsf(n.z);
if (inv_l1 < 1e-8f) inv_l1 = 1e-8f;
float x = n.x / inv_l1;
float y = n.y / inv_l1;
gbuffer_oct o;
if (n.z < 0.0f) {
        // fold the lower hemisphere up over the diagonal
        o.u = (1.0f - fabsf(y)) * sign_nz(x);
        o.v = (1.0f - fabsf(x)) * sign_nz(y);
    } else {
        o.u = x;
o.v = y;
}
    return o;
}

vec3 gbuffer_normal_decode(gbuffer_oct o) {
    vec3 n;
    n.x = o.u;
    n.y = o.v;
    n.z = 1.0f - (fabsf(o.u) + fabsf(o.v));

    if (n.z < 0.0f) {
        float ox = n.x;
        float oy = n.y;
        n.x = (1.0f - fabsf(oy)) * sign_nz(ox);
        n.y = (1.0f - fabsf(ox)) * sign_nz(oy);
    }

    float len = sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
    if (len < 1e-8f) len = 1e-8f;
    n.x /= len; n.y /= len; n.z /= len;
    return n;
}

// map [-1,1] -> [0, 1023] (10 bit). round to nearest.
static uint32_t enc10(float v) {
    float t = (v * 0.5f + 0.5f) * 1023.0f;
int i = (int)lrintf(t);
if (i < 0)    i = 0;
if (i > 1023) i = 1023;
return (uint32_t)i;
uint32_t u = enc10(o.u);
uint32_t v = enc10(o.v);
uint32_t a = (uint32_t)(face & 0x3);
return (a << 30) | (0u << 20) | (v << 10) | u;
