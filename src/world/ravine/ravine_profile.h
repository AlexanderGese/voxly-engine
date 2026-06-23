#ifndef WORLD_RAVINE_PROFILE_H
#define WORLD_RAVINE_PROFILE_H

#include "ravine_types.h"

// the cross-section profile. given how far a column sits from the centreline
// (and the local half-width), this maps to a cut depth — how far the canyon
// eats below the original surface here — and classifies the column into a
// ravine_cellkind. the silhouette:
//
// depth
// ^      ____               ____
// |     /    .    floor    .    .      <- rims taper the lip out
// |    / wall  .__________. wall .
// |   /                          .
// +--+----+--------------+----+----+--> distance from centreline
// rim  wall          floor wall rim
//
// the floor is a flat band of full max_depth out to half_width. past that the
// wall climbs back to the surface at wall_slope (run per rise), and a short rim
// past the wall feathers the very lip so the canyon edge isnt a sheer artefact.

typedef struct {
    int   cut;       // depth below surface to carve here (>=0)
    int   kind;      // ravine_cellkind
    float wall_t;    // 0..1 up the wall band; only meaningful when kind==WALL
} ravine_profile;

// evaluate the cross-section. dist is perpendicular distance to the centreline
// (cell units), half_width is the local floor half-width, p carries the depth +
// slope knobs. cut==0 / kind OUTSIDE means "leave the column alone".
ravine_profile ravine_profile_eval(float dist, float half_width,
                                   const ravine_params *p);

// horizontal run of the wall band for a given half_width: how far past the
// floor edge the wall reaches before it hits the surface. exposed because the
// mask bounds and the debug overlay both want it.
float ravine_profile_wall_run(const ravine_params *p);

#endif
