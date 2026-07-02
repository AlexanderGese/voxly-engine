#ifndef WORLD_WEATHERSIM_PRECIP_H
#define WORLD_WEATHERSIM_PRECIP_H

#include "weathersim_types.h"
#include "weathersim_field.h"
#include "weathersim_front.h"

// the precipitation pass. for each cell we work out the saturation humidity at
// its temperature (a clausius-clapeyron-ish curve), compare the actual humidity
// against it, and anything over the line condenses out and falls. the air loses
// that moisture (rain dries the column) and the ground cell banks it as
// accumulation. accumulation bleeds off slowly so puddles linger after a storm.
//
// the type of precip is a function of air temperature: warm -> rain, near
// freezing -> sleet, cold -> snow. intensity scales with how far over
// saturation we are *and* the front precip weight, so drizzle under a tired
// warm front and downpours under a punchy cold one come out naturally.

// saturation humidity (the max the air can hold) at a given temperature. it's a
// relative-humidity model so this returns the dew threshold the actual humidity
// is compared against, biased by params->dew_bias.
float weathersim_precip_saturation(float temp_c, float dew_bias);

// classify what falls at this air temperature and over-saturation amount.
weathersim_precip weathersim_precip_classify(float temp_c, float over,
                                             float snow_temp);

// run condensation across the whole field for one tick. mutates humidity (dries
// condensing cells), accumulates fallen precip into cell->accum, and bleeds old
// accumulation back off. front_weight comes from the front pool so storm cores
// rain harder than the bare thermodynamics would give. returns the fraction of
// cells actively precipitating, for telemetry.
float weathersim_precip_step(weathersim_field *f,
                             const weathersim_front_pool *fronts,
                             const weathersim_params *params, float dt);

// resolve the precip kind + intensity at a single grid cell, for the public
// query. reads the post-step field, doesn't mutate.
weathersim_precip weathersim_precip_at(const weathersim_field *f,
                                       const weathersim_params *params,
                                       int gx, int gz, float *intensity_out);

#endif
