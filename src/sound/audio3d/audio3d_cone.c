#include "audio3d_cone.h"

#include <math.h>

#ifndef AUDIO3D_DEG2RAD
#define AUDIO3D_DEG2RAD 0.01745329252f
#endif

void audio3d_cone_set(audio3d_voice *v, vec3 dir,
                      float inner_deg, float outer_deg, float outer_gain) {
    if (!v) return;
    if (inner_deg >= 360.0f) { audio3d_cone_clear(v); return; }
    if (outer_deg < inner_deg) outer_deg = inner_deg;   // outer must be wider
    if (outer_gain < 0.0f) outer_gain = 0.0f;
    if (outer_gain > 1.0f) outer_gain = 1.0f;

    vec3 d = vec3_normalize(dir);
    if (vec3_length_sq(d) < AUDIO3D_EPS) { audio3d_cone_clear(v); return; }

    // store the cosines of the half-angles. comparing cosines means we never
    // call acos in the hot path, just a dot product.
    v->cone_dir        = d;
    v->cone_inner      = cosf(0.5f * inner_deg * AUDIO3D_DEG2RAD);
    v->cone_outer      = cosf(0.5f * outer_deg * AUDIO3D_DEG2RAD);
    v->cone_outer_gain = outer_gain;
}

void audio3d_cone_clear(audio3d_voice *v) {
    if (!v) return;
    v->cone_dir        = VEC3_ZERO;
    v->cone_inner      = 0.0f;
    v->cone_outer      = 0.0f;
    v->cone_outer_gain = 1.0f;
}

float audio3d_cone_gain(const audio3d_voice *v, vec3 listener_pos) {
    if (!v) return 1.0f;
    if (vec3_length_sq(v->cone_dir) < AUDIO3D_EPS) return 1.0f;  // omni

    // angle between the cone axis and the direction to the listener.
    vec3 to_listener = vec3_sub(listener_pos, v->pos);
    if (vec3_length_sq(to_listener) < AUDIO3D_EPS) return 1.0f;  // on top of it
    vec3 nl = vec3_normalize(to_listener);

    float c = vec3_dot(v->cone_dir, nl);   // cos of the off-axis angle

    if (c >= v->cone_inner) return 1.0f;            // inside inner cone
    if (c <= v->cone_outer) return v->cone_outer_gain; // outside outer cone

    // between the cones: lerp on the cosine. not perfectly linear-in-angle but
    // close enough and a lot cheaper than two acos calls.
    float t = (c - v->cone_outer) / (v->cone_inner - v->cone_outer + AUDIO3D_EPS);
    return v->cone_outer_gain + (1.0f - v->cone_outer_gain) * t;
}
