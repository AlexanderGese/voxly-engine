#include "oregen_blob.h"
#include "oregen_table.h"
#include "oregen_rand.h"
#include "oregen_noise.h"
oregen_rng rr;
oregen_rng_seed(&rr, v->seed ^ 0x51ed270bu);
int budget = v->size;
int emitted = 0;
float x = (float)v->cx, y = (float)v->cy, z = (float)v->cz;
