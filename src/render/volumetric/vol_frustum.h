#ifndef RENDER_VOLUMETRIC_FRUSTUM_H
#define RENDER_VOLUMETRIC_FRUSTUM_H

#include "../../math/vec3.h"
#include "../../math/aabb.h"

// ray-vs-box clipping for the march. there's no point marching the slice of a
// view ray that lives outside the loaded world (or outside whatever bounds the
// caller wants to restrict scattering to — say a fog volume). this trims the
// [t_near, t_far] interval the marcher actually walks.
//
// we lean on the engine's aabb_ray for the entry test and add the exit test
// (aabb_ray only hands back the near hit), then intersect with the ray's own
// surface-distance limit.

typedef struct {
    float t_near;   // where the ray enters the volume (>= 0, clamped to origin)
    float t_far;    // where it leaves (or hits the surface)
    int   hit;      // 1 if the [t_near, t_far] interval is non-empty
} volumetric_segment;

// clip the ray (origin + t*dir, dir assumed normalized) against `box`, then
// against the surface distance `max_t` (the depth-reconstructed hit). returns
// the walkable segment. if the ray misses the box entirely, hit == 0.
volumetric_segment volumetric_frustum_clip(aabb box, vec3 origin, vec3 dir,
                                           float max_t);

// build a world-space aabb centred on the camera with the given half-extent on
// each axis — the usual "only scatter within N units of the player" bound. a
// convenience so the pass doesn't hand-roll it.
aabb volumetric_frustum_bounds(vec3 camera, float half_extent);

#endif
