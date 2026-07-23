#include "vol_selftest.h"
#include "vol_phase.h"
#include "vol_raymarch.h"
#include "vol_frustum.h"
#include "vol_dither.h"

#include <math.h>
#include <stdio.h>
#include <stddef.h>

#define VOL_PI 3.14159265358979323846f

// always-lit / always-shadowed shadow closures for the march tests.
static int sh_lit(void *u, vec3 p)    { (void)u; (void)p; return 1; }
static int sh_dark(void *u, vec3 p)   { (void)u; (void)p; return 0; }

int volumetric_selftest_phase_normalized(void) {
    // numerically integrate the hg phase over the sphere: it should be ~1.
    // sphere integral of f(cos) is 2*pi * integral_{-1}^{1} f(mu) dmu.
    const float gs[] = { 0.0f, 0.4f, -0.3f, 0.76f };
    const int N = 2048;
    for (int gi = 0; gi < 4; gi++) {
        float g = gs[gi];
        float acc = 0.0f;
        for (int i = 0; i < N; i++) {
            float mu = -1.0f + 2.0f * ((float)i + 0.5f) / (float)N;
            acc += volumetric_phase_hg(g, mu);
        }
        float dmu = 2.0f / (float)N;
        float integral = 2.0f * VOL_PI * acc * dmu;
        if (fabsf(integral - 1.0f) > 0.02f) {
            printf("[vol selftest] phase not normalized: g=%.2f int=%.4f\n",
                   g, integral);
            return 0;
        }
    }
    return 1;
}

int volumetric_selftest_transmittance(void) {
    if (fabsf(volumetric_transmittance(0.05f, 0.0f) - 1.0f) > 1e-6f) {
        printf("[vol selftest] T(0) != 1\n");
        return 0;
    }
    float prev = 1.0f;
    for (float d = 1.0f; d <= 50.0f; d += 1.0f) {
        float t = volumetric_transmittance(0.05f, d);
        if (t > prev + 1e-6f || t < 0.0f || t > 1.0f) {
            printf("[vol selftest] T not monotone at d=%.1f (%.4f > %.4f)\n",
                   d, t, prev);
            return 0;
        }
        prev = t;
    }
    return 1;
}

int volumetric_selftest_medium_clamp(void) {
    volumetric_medium m;
    // hand it garbage: extinct < scatter, g past the cap, a negative coeff.
    volumetric_medium_init(&m, 5.0f, 0.2f, 0.05f);
    if (m.extinct < m.scatter) {
        printf("[vol selftest] extinct < scatter after init\n");
        return 0;
    }
    if (!(fabsf(m.g) < 1.0f)) {
        printf("[vol selftest] g not clamped: %.3f\n", m.g);
        return 0;
    }
    volumetric_medium_init(&m, 0.0f, -1.0f, -2.0f);
    if (m.scatter < 0.0f || m.extinct < 0.0f) {
        printf("[vol selftest] negative coeff survived\n");
        return 0;
    }
    return volumetric_medium_valid(&m);
}

static volumetric_march_ctx base_ctx(volumetric_shadow_fn sh) {
    volumetric_march_ctx c;
    volumetric_medium_init(&c.medium, 0.5f, 0.04f, 0.06f);
    c.to_sun = vec3_new(0.0f, 1.0f, 0.0f);
    c.sun_color = vec3_new(1.0f, 1.0f, 1.0f);
    c.steps = 96;
    c.max_dist = 100.0f;
    c.dither = NULL;
    c.shadow = sh;
    c.shadow_user = NULL;
    return c;
}

int volumetric_selftest_march_energy(void) {
    volumetric_march_ctx c = base_ctx(sh_lit);
    float T;
    vec3 L = volumetric_march_ray(&c, vec3_new(0,0,0), vec3_new(0,0,50), 3, 7, &T);

    // the single-scatter ceiling: even fully lit, accumulated radiance can't
    // exceed sun_color * (scatter/extinct) (the analytic infinite-march limit).
    float ceiling = c.medium.scatter / c.medium.extinct;
    if (L.x < 0.0f || L.x > ceiling + 1e-3f) {
        printf("[vol selftest] march energy out of range: %.4f vs %.4f\n",
               L.x, ceiling);
        return 0;
    }
    if (T < 0.0f || T > 1.0f) {
        printf("[vol selftest] march transmittance bad: %.4f\n", T);
        return 0;
    }
    return 1;
}

int volumetric_selftest_march_shadowed(void) {
    volumetric_march_ctx c = base_ctx(sh_dark);
    float T;
    vec3 L = volumetric_march_ray(&c, vec3_new(0,0,0), vec3_new(0,0,50), 0, 0, &T);
    if (L.x != 0.0f || L.y != 0.0f || L.z != 0.0f) {
        printf("[vol selftest] shadowed march emitted light: %.4f\n", L.x);
        return 0;
    }
    // but the medium still absorbed along the ray.
    if (T >= 1.0f) {
        printf("[vol selftest] shadowed march didn't attenuate: %.4f\n", T);
        return 0;
    }
    return 1;
}

int volumetric_selftest_frustum_clip(void) {
    aabb box = aabb_make(vec3_new(-1,-1,-1), vec3_new(1,1,1));

    // ray from -z toward +z through the centre: enters at z=-1, exits at z=1.
    vec3 o = vec3_new(0,0,-5);
    vec3 d = vec3_new(0,0,1);
    volumetric_segment s = volumetric_frustum_clip(box, o, d, 100.0f);
    if (!s.hit || fabsf(s.t_near - 4.0f) > 1e-3f || fabsf(s.t_far - 6.0f) > 1e-3f) {
        printf("[vol selftest] clip centre wrong: hit=%d near=%.3f far=%.3f\n",
               s.hit, s.t_near, s.t_far);
        return 0;
    }

    // surface distance caps t_far short of the box exit.
    volumetric_segment s2 = volumetric_frustum_clip(box, o, d, 4.5f);
    if (!s2.hit || s2.t_far > 4.5f + 1e-3f) {
        printf("[vol selftest] clip max_t cap failed: far=%.3f\n", s2.t_far);
        return 0;
    }

    // a ray that sails past the box reports no hit.
    vec3 o3 = vec3_new(10,10,-5);
    volumetric_segment s3 = volumetric_frustum_clip(box, o3, d, 100.0f);
    if (s3.hit) {
        printf("[vol selftest] clip miss reported a hit\n");
        return 0;
    }
    return 1;
}

int volumetric_selftest_dither_range(void) {
    volumetric_dither d;
    volumetric_dither_build(&d);
    int n = d.dim * d.dim;
    for (int i = 0; i < n; i++) {
        if (d.texels[i] < 0.0f || d.texels[i] >= 1.0f) {
            printf("[vol selftest] dither out of [0,1): %.4f\n", d.texels[i]);
            return 0;
        }
    }
    // bayer values should be distinct (a permutation of the grid). check pairwise
    // — the tile is tiny so o(n^2) is fine.
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (fabsf(d.texels[i] - d.texels[j]) < 1e-6f) {
                printf("[vol selftest] dither has duplicate values\n");
                return 0;
            }
        }
    }
    return 1;
}

int volumetric_selftest_run_all(void) {
    int fails = 0;
    fails += !volumetric_selftest_phase_normalized();
    fails += !volumetric_selftest_transmittance();
    fails += !volumetric_selftest_medium_clamp();
    fails += !volumetric_selftest_march_energy();
    fails += !volumetric_selftest_march_shadowed();
    fails += !volumetric_selftest_frustum_clip();
    fails += !volumetric_selftest_dither_range();
    if (fails == 0) printf("[vol selftest] all passed\n");
    else            printf("[vol selftest] %d failure(s)\n", fails);
    return fails;
}
