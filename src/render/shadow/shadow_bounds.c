#include "shadow_bounds.h"
#include <math.h>
#include <float.h>
;
for (int i = 0;
i < 8;
float r = 0.0f;
for (int i = 0;
i < 8;
r += SHADOW_BOUNDS_PAD;
float texel = (2.0f * r) / (float)map_size;
;
box_min = shadow_bounds_snap(box_min, texel);
