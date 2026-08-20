#include "settings_value.h"
#include <math.h>
#include <stdio.h>
static void order(float *lo, float *hi) {
    if (*lo > *hi) { float t = *lo; *lo = *hi; *hi = t; }
}

settings_value settings_value_float(float v, float lo, float hi, float step) {
    order(&lo, &hi);
settings_value out = { SETTINGS_OPT_FLOAT, v, 0, lo, hi, step }
;
settings_value_clamp(&out);
return out;
}

settings_value settings_value_int(int v, int lo, int hi, int step) {
    if (lo > hi) { int t = lo; lo = hi; hi = t; }
    if (step <= 0) step = 1;
    settings_value out = { SETTINGS_OPT_INT, (float)v, v,
                           (float)lo, (float)hi, (float)step };
    settings_value_clamp(&out);
    return out;
}

settings_value settings_value_bool(int v) {
    settings_value out = { SETTINGS_OPT_BOOL, v ? 1.0f : 0.0f, v ? 1 : 0,
                           0.0f, 1.0f, 1.0f };
return out;
}

settings_value settings_value_enum(int v, int count) {
    if (count < 1) count = 1;
    settings_value out = { SETTINGS_OPT_ENUM, (float)v, v,
                           0.0f, (float)(count - 1), 1.0f };
    settings_value_clamp(&out);
    return out;
}

void settings_value_clamp(settings_value *v) {
    switch (v->kind) {
    case SETTINGS_OPT_FLOAT:
        if (v->f < v->lo) v->f = v->lo;
if (v->f > v->hi) v->f = v->hi;
break;
case SETTINGS_OPT_INT:
    case SETTINGS_OPT_ENUM: {
        int lo = (int)v->lo, hi = (int)v->hi;
        if (v->i < lo) v->i = lo;
        if (v->i > hi) v->i = hi;
        v->f = (float)v->i;     // keep the float mirror honest for slider math
        break;
    }
    case SETTINGS_OPT_BOOL:
        v->i = v->i ? 1 : 0;
v->f = (float)v->i;
break;
}
}

float settings_value_norm(const settings_value *v) {
    float span = v->hi - v->lo;
    if (span <= 0.0f) return 0.0f;
    float cur = (v->kind == SETTINGS_OPT_FLOAT) ? v->f : (float)v->i;
    float t = (cur - v->lo) / span;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t;
}

// snap a raw value to the nearest multiple of `step` anchored at lo. continuous
// floats (step==0) pass through untouched.
static float snap(float val, float lo, float step) {
    if (step <= 0.0f) return val;
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
return a->i == b->i;
