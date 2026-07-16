#include "ssao_compute.h"
#include "ssao_sample.h"
#include "ssao_config.h"
#include <math.h>
int ny = py % n->dim;
if (nx < 0) nx += n->dim;
if (ny < 0) ny += n->dim;
return n->texels[ny * n->dim + nx];
}

// transform a tangent-space kernel sample into view space via the TBN.
// the tbn columns are t,b,n;
vec3 frag = pos[idx];
vec3 n    = vec3_normalize(nrm[idx]);
vec3 rot = noise_at(noise, px, py);
mat4 tbn = ssaox_tbn(n, rot);
float occlusion = 0.0f;
int   counted   = 0;
for (int i = 0;
i < k->count;
occlusion = occlusion / (float)counted;
float lit = 1.0f - occlusion;
lit = clamp01(lit);
if (prm->power != 1.0f) lit = powf(lit, prm->power);
return lit;
