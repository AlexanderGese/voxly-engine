#ifndef WORLD_SKY_FOG_DENSITY_H
#define WORLD_SKY_FOG_DENSITY_H

// distance fog parameters derived from time of day + weather. the renderer
// can plug start/end (linear) or density (exp) straight into a shader.

typedef struct {
    float start;     // distance where fog begins (linear)
    float end;       // distance where fog is full (linear)
    float density;   // for exp2 fog
} voxl_sky_fog;

// base view distance is the clear-weather far plane in blocks.
voxl_sky_fog voxl_sky_fog_params(float hour, float wetness, float view_dist);

// exp2 fog factor at a given distance: 1 = clear, 0 = fully fogged.
float voxl_sky_fog_factor(float distance, float density);

#endif
