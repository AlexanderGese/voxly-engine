#ifndef SOUND_AUDIO3D_OCCLUSION_H
#define SOUND_AUDIO3D_OCCLUSION_H
#include "audio3d_types.h"
// occlusion estimate: how muffled is a source given the solid blocks between it
// and the ears. we dont want a full raycast per voice per frame (too pricey),
// so this walks a coarse line from listener to source and counts solids, then
// maps the count to a 0..1 muffle factor.
// callback the world provides: return non-zero if the block at (x,y,z) is solid
// (occludes sound). keeps this module from depending on world.h directly.
typedef int (*audio3d_solid_fn)(int x, int y, int z, void *user);
typedef struct {
    audio3d_solid_fn solid;
    void            *user;
    int              max_steps;   // safety cap on the dda walk
    float            per_block;   // muffle added per solid block crossed
} audio3d_occluder;
void audio3d_occluder_init(audio3d_occluder *o, audio3d_solid_fn fn, void *user);
// walk listener->source, count solids, return occlusion 0..1.
// returns 0 immediately if no solid callback is wired (open-air fallback).
float audio3d_occlusion_estimate(const audio3d_occluder *o,
                                 vec3 listener, vec3 source);
#endif
