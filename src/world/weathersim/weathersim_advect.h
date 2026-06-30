#ifndef WORLD_WEATHERSIM_ADVECT_H
#define WORLD_WEATHERSIM_ADVECT_H
#include "weathersim_types.h"
#include "weathersim_field.h"
// transport. the wind doesn't just sit there looking pretty — it carries
// temperature and moisture with it, which is what makes weather *travel*. this
void weathersim_advect_step(weathersim_field *f, float dt);
#endif
