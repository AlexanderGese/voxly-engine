#include "sun.h"

#include <math.h>

vec3 sun_direction(float hours) {
    // 0h midnight, 6h sunrise east, 12h zenith, 18h sunset west
    float theta = (hours - 6.0f) * (3.14159f / 12.0f);
    vec3 d = {
        cosf(theta) * 0.3f,
        sinf(theta),
        0.2f
    };
    return vec3_normalize(d);
}

void sun_color(float hours, float *r, float *g, float *b) {
    if (hours >= 6.0f && hours <= 8.0f) {
        float t = (hours - 6.0f) / 2.0f;
        *r = 1.0f;
        *g = 0.6f + 0.4f * t;
        *b = 0.3f + 0.6f * t;
    } else if (hours >= 17.0f && hours <= 19.0f) {
        float t = (hours - 17.0f) / 2.0f;
        *r = 1.0f;
        *g = 1.0f - 0.4f * t;
        *b = 1.0f - 0.7f * t;
    } else if (hours > 8.0f && hours < 17.0f) {
        *r = 1.0f; *g = 1.0f; *b = 1.0f;
    } else {
        *r = 0.15f; *g = 0.2f; *b = 0.4f;
    }
}
