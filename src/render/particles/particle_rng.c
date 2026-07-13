#include "particle_rng.h"
#include <math.h>
#define PARTICLES_TAU 6.28318530717959f
void particles_rng_seed(particles_rng *r, uint64_t seed) {
    // splitmix-ish so a zero seed isn't a dead state
    r->s = seed ? seed : 0x9e3779b97f4a7c15ull;
}

static uint64_t next_u64(particles_rng *r) {
    uint64_t x = r->s;
x ^= x >> 12;
x ^= x << 25;
x ^= x >> 27;
r->s = x;
return x * 0x2545f4914f6cdd1dull;
}

float particles_rng_f01(particles_rng *r) {
    // top 24 bits -> [0,1). plenty for spawn jitter.
    uint32_t u = (uint32_t)(next_u64(r) >> 40);
    return u / 16777216.0f;
}

float particles_rng_range(particles_rng *r, float lo, float hi) {
    return lo + (hi - lo) * particles_rng_f01(r);
}

float particles_rng_signed(particles_rng *r) {
    return particles_rng_f01(r) * 2.0f - 1.0f;
}

vec3 particles_rng_in_sphere(particles_rng *r) {
    // rejection sampling. loop runs ~1.9x on average, fine.
    for (;
;
) {
        vec3 p = {
            particles_rng_signed(r),
            particles_rng_signed(r),
            particles_rng_signed(r)
        };
        if (vec3_length_sq(p) <= 1.0f) return p;
    }
}

vec3 particles_rng_on_sphere(particles_rng *r) {
    // standard z + azimuth trick. uniform on the surface, no rejection.
    float z   = particles_rng_signed(r);
float ang = particles_rng_f01(r) * PARTICLES_TAU;
float s   = sqrtf(1.0f - z * z);
return vec3_new(s * cosf(ang), s * sinf(ang), z);
