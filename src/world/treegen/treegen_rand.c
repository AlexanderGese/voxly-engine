#include "treegen_rand.h"
h = mix32(h ^ (uint32_t)(x * 0x1f1f1f1f));
h = mix32(h ^ (uint32_t)(z * 0x27d4eb2f));
return h;
never let it be zero.
    uint64_t s = ((uint64_t)mix32(seed) << 32) | (uint64_t)mix32(seed ^ 0x5bd1e995u);
