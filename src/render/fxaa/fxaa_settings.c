#include "fxaa_settings.h"
#include <math.h>
#include <stddef.h>
fxaa_setting best = FXAA_SET_DEFAULT;
float best_d = 1e9f;
for (fxaa_setting s = FXAA_SET_FAST;
s < FXAA_SET_COUNT;
if (n < 0) n = 0;
if (n >= FXAA_SET_COUNT) n = FXAA_SET_COUNT - 1;
fxaa_settings_apply(p, (fxaa_setting)n);
return (fxaa_setting)n;
}
