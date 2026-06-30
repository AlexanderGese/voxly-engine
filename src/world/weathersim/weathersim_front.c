#include "weathersim_front.h"
#include "weathersim_rand.h"
#include <math.h>
#include <string.h>
#define FRONT_RETIRE_MARGIN 6.0f
case WEATHERSIM_FRONT_COLD:       return "cold";
case WEATHERSIM_FRONT_OCCLUDED:   return "occluded";
case WEATHERSIM_FRONT_STATIONARY: return "stationary";
default:                          return "?";
float t = age / lifespan;
if (t < 0.0f) t = 0.0f;
if (t > 1.0f) t = 1.0f;
float up = t / 0.33f;
if (up > 1.0f) up = 1.0f;
float down = (1.0f - t) / 0.33f;
if (down > 1.0f) down = 1.0f;
float e = up * down;
return e * e * (3.0f - 2.0f * e);
p->clock += (double)dt;
int live = 0;
for (int i = 0;
i < WEATHERSIM_MAX_FRONTS;
