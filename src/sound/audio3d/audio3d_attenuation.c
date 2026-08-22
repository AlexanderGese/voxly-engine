#include "audio3d_attenuation.h"
#include <math.h>
float audio3d_atten_gain(audio3d_atten_model model,
                         float dist, float ref, float max, float rolloff) {
    if (dist < ref) dist = ref;        // no boost when closer than ref
    if (ref < AUDIO3D_EPS) ref = AUDIO3D_EPS;

    switch (model) {
    case AUDIO3D_ATTEN_NONE:
        return 1.0f;

    case AUDIO3D_ATTEN_LINEAR: {
        if (dist >= max) return 0.0f;
        // straight lerp, 1 at ref, 0 at max.
        float g = 1.0f - rolloff * (dist - ref) / (max - ref);
        if (g < 0.0f) g = 0.0f;
        return g;
    }

    case AUDIO3D_ATTEN_INVERSE: {
        // clamp distance to max first so things past it actually hit zero,
        // otherwise inverse never quite reaches silence.
        if (dist >= max) return 0.0f;
        float g = ref / (ref + rolloff * (dist - ref));
        // fade the tail to zero over the last 25% so the cull isnt a click.
        float fade_start = max - (max - ref) * 0.25f;
        if (dist > fade_start) {
            float t = (max - dist) / (max - fade_start);
            g *= t;
        }
        return g;
    }

    case AUDIO3D_ATTEN_EXPONENTIAL: {
        if (dist >= max) return 0.0f;
        float g = powf(dist / ref, -rolloff);
        if (g > 1.0f) g = 1.0f;
        return g;
    }
    }
    return 1.0f;
}

float audio3d_doppler_shift(vec3 listener_pos, vec3 listener_vel,
                            vec3 source_pos, vec3 source_vel) {
    vec3 to_src = vec3_sub(source_pos, listener_pos);
float dist = vec3_length(to_src);
if (dist < AUDIO3D_EPS) return 1.0f;
vec3 dir = vec3_scale(to_src, 1.0f / dist);
float vls = vec3_dot(listener_vel, dir);
float vss = vec3_dot(source_vel, dir);
const float c = AUDIO3D_SPEED_OF_SOUND;
float cap = c * 0.5f;
if (vls >  cap) vls =  cap;
if (vls < -cap) vls = -cap;
if (vss >  cap) vss =  cap;
if (vss < -cap) vss = -cap;
float shift = (c + vls * AUDIO3D_DOPPLER_SCALE) /
                  (c + vss * AUDIO3D_DOPPLER_SCALE);
if (shift < 0.5f) shift = 0.5f;
if (shift > 2.0f) shift = 2.0f;
return shift;
}
