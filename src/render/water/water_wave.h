#ifndef RENDER_WATER_WATER_WAVE_H
#define RENDER_WATER_WATER_WAVE_H
#include "../../math/vec2.h"
#include "../../math/vec3.h"
#include "water_config.h"
// gerstner wave field. each wave is a travelling sine that also pushes the
// surface horizontally toward the crest, which is what makes water look like
// water and not a wobbling bedsheet. we evaluate the sum on the cpu to get a
// surface normal for the fresnel term; the shader redoes the fine detail.
typedef struct {
    vec2  dir;          // unit direction of travel in the xz plane
    float wavelength;   // crest-to-crest distance in blocks
    float amplitude;    // peak height in blocks
    float steepness;    // 0..1, how pinched the crest is
    float speed;        // phase speed multiplier
    float phase;        // baked: 2*pi / wavelength
    float omega;        // baked: phase * speed
} water_wave;
typedef struct {
    water_wave waves[WATER_MAX_WAVES];
    int        count;
    float      amp_scale;   // global amplitude multiplier
    float      time;        // accumulated seconds
} water_wave_field;
// result of sampling the field at a point: the displaced position offset and
// the surface normal there.
typedef struct {
    vec3 offset;        // add to flat (x, surface_y, z) to get the wave surface
    vec3 normal;        // unit surface normal
} water_wave_sample;
// build a default field. a spread of directions and wavelengths that doesnt
// obviously repeat. seed picks the directions so two pools can differ.
void water_wave_field_init(water_wave_field *f, unsigned seed);
// push a single wave. returns 0 if the field is already full.
int  water_wave_field_add(water_wave_field *f, vec2 dir, float wavelength,
                          float amplitude, float steepness, float speed);
// advance the animation clock.
void water_wave_field_tick(water_wave_field *f, float dt);
// sample the summed field at world xz on a surface sitting at surface_y.
water_wave_sample water_wave_field_sample(const water_wave_field *f,
                                          float x, float z, float surface_y);
// just the height (cheaper, no normal) — handy for buoyancy/foam queries.
float water_wave_field_height(const water_wave_field *f, float x, float z);
// total energy of the field, roughly sum(amp^2). lets the shoreline foam
// scale with how choppy it is.
float water_wave_field_energy(const water_wave_field *f);
#endif
