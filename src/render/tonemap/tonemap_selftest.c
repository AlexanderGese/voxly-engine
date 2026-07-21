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
tonemap_exposure_init(&e);
tonemap_exposure_measure(&e, 0.01f);
auto_ev should converge close to target.
    for (int i = 0;
i < 600;
i++) tonemap_exposure_update(&e, 1.0f / 60.0f);
if (!tonemap_lut_make_identity(&l, 17)) return 0;
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
