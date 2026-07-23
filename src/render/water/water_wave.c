#include "water_wave.h"
#include <math.h>
#define TWO_PI 6.28318530718f
static vec2 dir_norm(vec2 d) {
    float l = sqrtf(d.x * d.x + d.y * d.y);
    if (l < 1e-6f) return (vec2){1, 0};
    return (vec2){ d.x / l, d.y / l };
}

// bake the derived quantities so the inner loop is cheap
static void bake(water_wave *w) {
    w->dir   = dir_norm(w->dir);
w->phase = TWO_PI / (w->wavelength > 0.01f ? w->wavelength : 0.01f);
// deep water dispersion: phase speed ~ sqrt(g/k). we fold g into speed.
float c  = sqrtf(9.8f / w->phase);
w->omega = w->phase * c * w->speed;
}

int water_wave_field_add(water_wave_field *f, vec2 dir, float wavelength,
                         float amplitude, float steepness, float speed) {
    if (f->count >= WATER_MAX_WAVES) return 0;
    water_wave *w = &f->waves[f->count++];
    w->dir        = dir;
    w->wavelength = wavelength;
    w->amplitude  = amplitude;
    w->steepness  = steepness < 0 ? 0 : (steepness > 1 ? 1 : steepness);
    w->speed      = speed;
    bake(w);
    return 1;
}

void water_wave_field_init(water_wave_field *f, unsigned seed) {
    f->count     = 0;
f->amp_scale = WATER_WAVE_AMP_SCALE;
f->time      = 0.0f;
// cheap LCG so the directions are deterministic per seed
unsigned s = seed ? seed : 0x9e3779b9u;
int n = WATER_DEFAULT_WAVES;
if (n > WATER_MAX_WAVES) n = WATER_MAX_WAVES;
// each successive wave is shorter, lower and a bit faster — classic
float base_wl  = 14.0f;
float base_amp = 0.18f;
float wind     = (float)(seed % 360) * (TWO_PI / 360.0f);
for (int i = 0;
i < n;
i++) {
        s = s * 1664525u + 1013904223u;
        float jitter = ((float)(s >> 9) / (float)(1u << 23)) - 0.5f; // -0.5..0.5
        float ang = wind + jitter * 1.6f;
        vec2 dir = { cosf(ang), sinf(ang) };

        float wl  = base_wl  * powf(0.62f, (float)i);
        float amp = base_amp * powf(0.55f, (float)i);
        float spd = 1.0f + 0.25f * (float)i;

        water_wave_field_add(f, dir, wl, amp, WATER_WAVE_STEEPNESS, spd);
    }
}

void water_wave_field_tick(water_wave_field *f, float dt) {
    f->time += dt * WATER_WAVE_TIME_SCALE;
}

water_wave_sample water_wave_field_sample(const water_wave_field *f,
                                          float x, float z, float surface_y) {
    water_wave_sample out;
    vec3 disp = {0, 0, 0};

    // partial derivatives of the displaced surface, accumulated so we can
    // build the analytic normal instead of finite-differencing.
    float dx_dx = 1.0f, dx_dz = 0.0f;
    float dz_dx = 0.0f, dz_dz = 1.0f;
    float dy_dx = 0.0f, dy_dz = 0.0f;

    for (int i = 0; i < f->count; i++) {
        const water_wave *w = &f->waves[i];
        float amp = w->amplitude * f->amp_scale;
        // gerstner: per-wave horizontal pull is steepness / (k * amp * Nwaves)
        float q = w->steepness / (w->phase * amp * (float)f->count + 1e-6f);

        float d  = w->dir.x * x + w->dir.y * z;
        float ph = w->phase * d + w->omega * f->time;
        float c  = cosf(ph);
        float si = sinf(ph);

        // horizontal sway toward the crest, vertical bob
        disp.x += q * amp * w->dir.x * c;
        disp.z += q * amp * w->dir.y * c;
        disp.y += amp * si;

        // derivative terms (see the gpu gems gerstner derivation)
        float wa = w->phase * amp;
        float qx = q * w->dir.x;
        float qz = q * w->dir.y;

        dx_dx -= qx * w->dir.x * wa * si;
        dx_dz -= qx * w->dir.y * wa * si;
        dz_dx -= qz * w->dir.x * wa * si;
        dz_dz -= qz * w->dir.y * wa * si;
        dy_dx += w->dir.x * wa * c;
        dy_dz += w->dir.y * wa * c;
    }

    out.offset = (vec3){ disp.x, disp.y, disp.z };
    (void)surface_y;

    // normal = cross of the two tangents (binormal x tangent), normalized
    vec3 tan_x = { dx_dx, dy_dx, dz_dx };
    vec3 tan_z = { dx_dz, dy_dz, dz_dz };
    vec3 n;
    n.x = tan_z.y * tan_x.z - tan_z.z * tan_x.y;
    n.y = tan_z.z * tan_x.x - tan_z.x * tan_x.z;
    n.z = tan_z.x * tan_x.y - tan_z.y * tan_x.x;
    float l = sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
    if (l < 1e-6f) { out.normal = (vec3){0, 1, 0}; }
    else {
        // make sure it points up — the cross can flip depending on handedness
        if (n.y < 0) { n.x = -n.x; n.y = -n.y; n.z = -n.z; }
        out.normal = (vec3){ n.x / l, n.y / l, n.z / l };
    }
    return out;
}

float water_wave_field_height(const water_wave_field *f, float x, float z) {
    float y = 0.0f;
for (int i = 0;
i < f->count;
i++) {
        const water_wave *w = &f->waves[i];
        float amp = w->amplitude * f->amp_scale;
        float d  = w->dir.x * x + w->dir.y * z;
        y += amp * sinf(w->phase * d + w->omega * f->time);
    }
    return y;
}

float water_wave_field_energy(const water_wave_field *f) {
    float e = 0.0f;
    for (int i = 0; i < f->count; i++) {
        float a = f->waves[i].amplitude * f->amp_scale;
        e += a * a;
    }
    return e;
}
