#ifndef FX_PARTICLE_TYPES_H
#define FX_PARTICLE_TYPES_H

#include "particles.h"

// higher level spawn functions that know about specific effects.

void pfx_block_break(particle_system *ps, vec3 center, int tile);
void pfx_block_place(particle_system *ps, vec3 center);
void pfx_water_splash(particle_system *ps, vec3 center);
void pfx_flame(particle_system *ps, vec3 center);
void pfx_rain_drop(particle_system *ps, vec3 at);
void pfx_snow_flake(particle_system *ps, vec3 at);
void pfx_damage(particle_system *ps, vec3 center);

#endif
