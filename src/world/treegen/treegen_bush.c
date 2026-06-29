#include "treegen_bush.h"
#include "treegen_rand.h"
treegen_rng r;
treegen_rng_seed(&r, seed);
int h = treegen_rng_range(&r, 1, 2);
