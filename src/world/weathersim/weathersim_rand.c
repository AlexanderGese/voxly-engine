#include "weathersim_rand.h"
h ^= (uint64_t)(uint32_t)z * 0xc2b2ae3d27d4eb4full;
h ^= (uint64_t)seed << 17;
return (uint32_t)(mix64(h) >> 32);
if (r->s == 0) r->s = 0xa5a5a5a5deadc0deull;
uint32_t span = (uint32_t)(hi - lo) + 1u;
return lo + (int)(weathersim_rng_next(r) % span);
for (int i = 0;
i < 4;
++i) s += weathersim_rng_f01(r);
return (s - 2.0f) * 0.8660254f;
}
