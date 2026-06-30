#ifndef WORLD_WEATHERSIM_ADVECT_H
#define WORLD_WEATHERSIM_ADVECT_H

#include "weathersim_types.h"
#include "weathersim_field.h"

// transport. the wind doesn't just sit there looking pretty — it carries
// temperature and moisture with it, which is what makes weather *travel*. this
// is a semi-lagrangian advection step: for each cell we trace backwards along
// the local wind for one tick, sample the field where the air *came from* (with
// bilinear blend), and that becomes the cell's new value.
//
// semi-lagrangian over a forward/eulerian scheme because it's unconditionally
// stable — you can't blow it up with a strong gust the way an explicit upwind
// finite-difference would, and at this grid resolution we can afford the
// backtrace + bilinear gather. it does diffuse a little (the bilinear smears),
// but the field already has a diffusion pass so that's a feature here.
//
// only humidity, temperature and cloud get advected. pressure is owned by the
// fronts (advecting it too would double-count their motion) and wind is
// re-solved from pressure every tick, so transporting it is pointless.

// advect the field along its own wind for one tick of length dt. uses the
// scratch buffer as the destination then commits. wind is in blocks/sec, so we
// convert to cells/tick internally using the cell size.
void weathersim_advect_step(weathersim_field *f, float dt);

#endif
