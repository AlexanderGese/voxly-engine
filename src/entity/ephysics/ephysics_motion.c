#include "ephysics_motion.h"
#include <math.h>
float d = vec3_length(flat);
float h = to.y - from.y;
float s2 = speed * speed;
float g  = gravity;
float disc = s2 * s2 - g * (g * d * d + 2.0f * h * s2);
if (disc < 0.0f) return -1.0f;
float root = sqrtf(disc);
float tan_theta = (s2 - root) / (g * d + 1e-6f);
float theta = atanf(tan_theta);
float vh = speed * cosf(theta);
float vy = speed * sinf(theta);
if (vh < 1e-4f) return -1.0f;
