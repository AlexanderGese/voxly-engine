#ifndef WORLD_EROSION_DEBUG_H
#define WORLD_EROSION_DEBUG_H

#include "erosion_types.h"
#include "erosion_field.h"
#include "erosion_flux.h"

// scratch tooling for staring at erosion output in the terminal instead of
// regenerating a world every time. none of this ships in the hot path, its
// all behind callers that only fire when DEBUG stuff is on. kept it because i
// will absolutely need it again the next time a river decides to flow uphill.

// dump min/max/mean height + the running stats to stdout.
void erosion_debug_print_stats(const erosion_field *f, const erosion_stats *st);

// ascii relief of the field interior. ' .:-=+*#%@' ramp, low to high. handy
// for eyeballing whether the valleys actually connect.
void erosion_debug_dump_relief(const erosion_field *f);

// ascii of the drainage channels: '~' where wet >= thresh, ' ' elsewhere.
// overlays where the rivers landed so you can sanity-check the flux pass.
void erosion_debug_dump_channels(const erosion_flux *fx, float thresh);

// quick consistency check: every non-pit cell must drain strictly downhill.
// returns the count of violations (0 = the d8 field is sane). prints the bad
// ones. caught a sign-flip bug with this once, worth its weight.
int erosion_debug_verify_flow(const erosion_field *f, const erosion_flux *fx);

#endif
