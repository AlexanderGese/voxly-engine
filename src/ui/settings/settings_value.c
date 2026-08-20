#include "settings_value.h"

#include <math.h>
#include <stdio.h>

static void order(float *lo, float *hi) {
    if (*lo > *hi) { float t = *lo; *lo = *hi; *hi = t; }
}

settings_value settings_value_float(float v, float lo, float hi, float step) {
    order(&lo, &hi);
    settings_value out = { SETTINGS_OPT_FLOAT, v, 0, lo, hi, step };
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
}

int settings_value_set_norm(settings_value *v, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    float raw = v->lo + t * (v->hi - v->lo);

    if (v->kind == SETTINGS_OPT_FLOAT) {
        float snapped = snap(raw, v->lo, v->step);
        if (fabsf(snapped - v->f) < 1e-6f) return 0;
        v->f = snapped;
        settings_value_clamp(v);
        return 1;
    }

    int ni = (int)floorf(raw + 0.5f);
    if (ni == v->i) return 0;
    v->i = ni;
    settings_value_clamp(v);
    return 1;
}

int settings_value_step(settings_value *v, int dir) {
    if (dir == 0) return 0;
    if (v->kind == SETTINGS_OPT_FLOAT) {
        float step = v->step > 0.0f ? v->step : (v->hi - v->lo) * 0.05f;
        float nv = v->f + (dir > 0 ? step : -step);
        if (nv < v->lo) nv = v->lo;
        if (nv > v->hi) nv = v->hi;
        if (fabsf(nv - v->f) < 1e-6f) return 0;
        v->f = nv;
        return 1;
    }
    // int / bool / enum all advance by one index here
    int step = (int)(v->step > 0 ? v->step : 1);
    int nv = v->i + (dir > 0 ? step : -step);
    int lo = (int)v->lo, hi = (int)v->hi;
    if (nv < lo) nv = lo;
    if (nv > hi) nv = hi;
    if (nv == v->i) return 0;
    v->i = nv;
    v->f = (float)nv;
    return 1;
}

int settings_value_cycle(settings_value *v, int dir) {
    if (v->kind == SETTINGS_OPT_BOOL) {
        int nv = !v->i;
        if (nv == v->i) return 0;
        v->i = nv; v->f = (float)nv;
        return 1;
    }
    if (v->kind == SETTINGS_OPT_ENUM) {
        int lo = (int)v->lo, hi = (int)v->hi;
        int span = hi - lo + 1;
        if (span < 1) return 0;
        int nv = v->i + (dir >= 0 ? 1 : -1);
        // wrap around — that's the whole point of a cycler
        if (nv > hi) nv = lo;
        if (nv < lo) nv = hi;
        if (nv == v->i) return 0;
        v->i = nv; v->f = (float)nv;
        return 1;
    }
    // not a cyclable kind, fall back to a single step
    return settings_value_step(v, dir);
}

int settings_value_equal(const settings_value *a, const settings_value *b) {
    if (a->kind != b->kind) return 0;
    if (a->kind == SETTINGS_OPT_FLOAT)
        return fabsf(a->f - b->f) < 1e-5f;
    return a->i == b->i;
}

void settings_value_format(const settings_value *v, const char *const *choices,
                           int choice_count, char *out, size_t out_sz) {
    if (!out || out_sz == 0) return;
    switch (v->kind) {
    case SETTINGS_OPT_FLOAT:
        // volumes read nicer as a percentage, but we don't know semantics here;
        // keep two decimals and let the schema pick a friendlier label if it
        // ever cares. good enough, ship it.
        snprintf(out, out_sz, "%.2f", v->f);
        break;
    case SETTINGS_OPT_INT:
        snprintf(out, out_sz, "%d", v->i);
        break;
    case SETTINGS_OPT_BOOL:
        snprintf(out, out_sz, "%s", v->i ? "on" : "off");
        break;
    case SETTINGS_OPT_ENUM:
        if (choices && v->i >= 0 && v->i < choice_count && choices[v->i])
            snprintf(out, out_sz, "%s", choices[v->i]);
        else
            snprintf(out, out_sz, "#%d", v->i);
        break;
    default:
        out[0] = '\0';
        break;
    }
}
