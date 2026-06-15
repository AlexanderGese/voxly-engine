#ifndef WORLD_EROSION_DROPLET_H
#define WORLD_EROSION_DROPLET_H

#include <stdint.h>
#include "erosion_types.h"
#include "erosion_field.h"

// the hydraulic erosion pass: rain droplets. each droplet is a marble that
// spawns somewhere on the tile, rolls downhill picking up momentum, carries
// sediment up to a slope-dependent capacity, carves where it has room and
// deposits where it's overloaded, then evaporates. classic particle-based
// erosion (mei/jakob-ish), no full shallow-water grid because this runs at
// worldgen time and we cant afford the iterations.

// spawn one droplet at a (jittered) interior cell from its private stream.
erosion_droplet erosion_droplet_spawn(const erosion_field *f,
                                      const erosion_params *p,
                                      int droplet_index);

// run a single droplet to death, mutating the field. returns 1 if it died
// on-map (evaporated / stalled), 0 if it ran off the skirt. accumulates into
// stats if non-NULL.
int erosion_droplet_run(erosion_field *f, const erosion_params *p,
                        erosion_droplet *d, erosion_stats *st);

// spawn + run `p->droplets` of them. the main entry the driver calls.
void erosion_hydraulic_pass(erosion_field *f, const erosion_params *p,
                            erosion_stats *st);

#endif
