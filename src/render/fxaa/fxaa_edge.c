#include "fxaa_edge.h"
#include "fxaa_luma.h"
#include <math.h>
info->horizontal = 1;
info->contrast = 0.0f;
info->blend_subpix = 0.0f;
float c = sample(0, 0, user);
float n = sample(0, -1, user);
float s = sample(0,  1, user);
float w = sample(-1, 0, user);
