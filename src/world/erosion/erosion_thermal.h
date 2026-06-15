#ifndef WORLD_EROSION_THERMAL_H
#define WORLD_EROSION_THERMAL_H
#include "erosion_types.h"
#include "erosion_field.h"
// thermal / talus erosion: the dry counterpart to the droplet pass. wherever
// the height difference between a cell and a downhill neighbour exceeds the
float erosion_thermal_sweep(erosion_field *f, const erosion_params *p);
void erosion_thermal_pass(erosion_field *f, const erosion_params *p,
                          erosion_stats *st);
#endif
