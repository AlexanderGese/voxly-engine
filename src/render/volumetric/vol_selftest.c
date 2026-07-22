#include "vol_selftest.h"
#include "vol_phase.h"
#include "vol_raymarch.h"
#include "vol_frustum.h"
#include "vol_dither.h"
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#define VOL_PI 3.14159265358979323846f
(void)p;
return 0;
return 0;
}
    float prev = 1.0f;
for (float d = 1.0f;
d <= 50.0f;
volumetric_medium_init(&c.medium, 0.5f, 0.04f, 0.06f);
c.to_sun = vec3_new(0.0f, 1.0f, 0.0f);
c.sun_color = vec3_new(1.0f, 1.0f, 1.0f);
c.steps = 96;
c.max_dist = 100.0f;
c.dither = NULL;
c.shadow = sh;
c.shadow_user = NULL;
return c;
float T;
