#include "settings_value.h"
#include <math.h>
#include <stdio.h>
;
settings_value_clamp(&out);
return out;
return out;
if (v->f > v->hi) v->f = v->hi;
break;
v->f = (float)v->i;
break;
float n = floorf((val - lo) / step + 0.5f);
return lo + n * step;
int nv = v->i + (dir > 0 ? step : -step);
int lo = (int)v->lo, hi = (int)v->hi;
if (nv < lo) nv = lo;
if (nv > hi) nv = hi;
if (nv == v->i) return 0;
v->i = nv;
v->f = (float)nv;
return 1;
if (a->kind == SETTINGS_OPT_FLOAT)
        return fabsf(a->f - b->f) < 1e-5f;
