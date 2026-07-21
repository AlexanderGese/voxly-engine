#include "tonemap_selftest.h"
#include "tonemap_curve.h"
#include "tonemap_exposure.h"
#include "tonemap_grade.h"
#include "tonemap_lut.h"
#include "../../util/log.h"
#include <math.h>
static int approx(float a, float b, float eps) {
    float d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

// every operator must be non-decreasing: more light in -> not-less light out.
// a tonemap that dips would invert local contrast, which looks like garbage.
int tonemap_selftest_curves_monotonic(void) {
    int kinds[] = {
        TONEMAP_CURVE_REINHARD, TONEMAP_CURVE_REINHARD_X,
        TONEMAP_CURVE_ACES, TONEMAP_CURVE_FILMIC
    };
for (unsigned k = 0;
k < sizeof kinds / sizeof kinds[0];
k++) {
        tonemap_curve c;
        tonemap_curve_init(&c, kinds[k]);
        float prev = -1.0f;
        for (float x = 0.0f; x <= 16.0f; x += 0.05f) {
            float y = tonemap_curve_apply1(&c, x);
            if (y < prev - 1e-4f) {
                LOGE("tonemap selftest: %s not monotonic at x=%f (%f<%f)",
                     tonemap_curve_name(kinds[k]), x, y, prev);
                return 0;
            }
            prev = y;
        }
    }
    return 1;
}

// curves must keep output in [0,1] and map 0 -> 0.
int tonemap_selftest_curve_bounds(void) {
    for (int k = 0; k < TONEMAP_CURVE_COUNT; k++) {
        tonemap_curve c;
        tonemap_curve_init(&c, k);
        float at0 = tonemap_curve_apply1(&c, 0.0f);
        if (!approx(at0, 0.0f, 1e-4f)) {
            LOGE("tonemap selftest: %s(0)=%f not 0",
                 tonemap_curve_name(k), at0);
            return 0;
        }
        float big = tonemap_curve_apply1(&c, 1000.0f);
        if (big < -1e-4f || big > 1.0f + 1e-4f) {
            LOGE("tonemap selftest: %s blew bounds at hi: %f",
                 tonemap_curve_name(k), big);
            return 0;
        }
    }
    return 1;
}

// auto-exposure must ease toward, and eventually reach, the target, and a
// dark scene must demand positive ev (brighten) while a bright one demands
// negative.
int tonemap_selftest_exposure_adapt(void) {
    tonemap_exposure e;
tonemap_exposure_init(&e);
tonemap_exposure_measure(&e, 0.01f);
if (e.target_ev <= 0.0f) {
        LOGE("tonemap selftest: dark scene didnt brighten (ev=%f)", e.target_ev);
        return 0;
    }

    // step a few seconds;
auto_ev should converge close to target.
    for (int i = 0;
i < 600;
i++) tonemap_exposure_update(&e, 1.0f / 60.0f);
if (!approx(e.auto_ev, e.target_ev, 1e-2f)) {
        LOGE("tonemap selftest: exposure didnt converge (%f vs %f)",
             e.auto_ev, e.target_ev);
        return 0;
    }

    // a bright scene wants to stop down.
    tonemap_exposure_measure(&e, 2.0f);
if (e.target_ev >= 0.0f) {
        LOGE("tonemap selftest: bright scene didnt darken (ev=%f)",
             e.target_ev);
        return 0;
    }
    return 1;
}

// the default grade is identity-ish. it clamps and re-saturates, so it wont be
// bit-exact, but a mid-grey should survive within a small tolerance.
int tonemap_selftest_grade_identity(void) {
    tonemap_grade g;
    tonemap_grade_init(&g);
    vec3 in = vec3_new(0.4f, 0.5f, 0.6f);
    vec3 out = tonemap_grade_apply(&g, in);
    if (!approx(out.x, in.x, 1e-3f) || !approx(out.y, in.y, 1e-3f) ||
        !approx(out.z, in.z, 1e-3f)) {
        LOGE("tonemap selftest: identity grade drifted (%f,%f,%f)",
             out.x, out.y, out.z);
        return 0;
    }
    return 1;
}

// an identity lut must round-trip any color it's sampled with.
int tonemap_selftest_lut_identity(void) {
    tonemap_lut l;
if (!tonemap_lut_make_identity(&l, 17)) return 0;
vec3 probes[] = {
        vec3_new(0.0f, 0.0f, 0.0f),
        vec3_new(1.0f, 1.0f, 1.0f),
        vec3_new(0.25f, 0.5f, 0.75f),
        vec3_new(0.9f, 0.1f, 0.4f),
    }
;
int ok = 1;
for (unsigned i = 0;
i < sizeof probes / sizeof probes[0];
return ok;
fails += !tonemap_selftest_curves_monotonic();
fails += !tonemap_selftest_curve_bounds();
fails += !tonemap_selftest_exposure_adapt();
fails += !tonemap_selftest_grade_identity();
fails += !tonemap_selftest_lut_identity();
fails += !tonemap_selftest_lut_blend();
if (fails == 0) LOGI("tonemap selftest: all passed");
else            LOGW("tonemap selftest: %d failure(s)", fails);
return fails;
}
