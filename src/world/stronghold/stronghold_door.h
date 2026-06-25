#ifndef WORLD_STRONGHOLD_DOOR_H
#define WORLD_STRONGHOLD_DOOR_H
#include "stronghold_graph.h"
#include "stronghold_buffer.h"
#include "stronghold_rand.h"
// door decorator. corridors leave 1x2 holes and record them; this pass dresses
// each hole into one of the door kinds (open arch, plank door, iron bars, low
// crawl grate) based on the rooms it joins and a per-door roll. purely cosmetic
// over the carve, so it must run after corridors.
// classify a door given the rooms either side. portal/prison get bars, library
// gets plank doors, the rest are mostly open arches with the odd plank.
stronghold_door_kind stronghold_door_classify(const stronghold_room *r,
                                              const stronghold_door *d,
                                              stronghold_rng *rng);
// dress every recorded door in every room. returns voxels emitted.
int stronghold_door_decorate_all(const stronghold_graph *g, stronghold_buffer *out,
                                 stronghold_rng *rng);
int stronghold_door_decorate(const stronghold_room *r, const stronghold_door *d,
                             stronghold_door_kind kind, stronghold_buffer *out);
#endif
