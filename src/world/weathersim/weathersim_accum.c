#include "weathersim_accum.h"
#include "weathersim_precip.h"

#include <math.h>

// where the snow line sits per layer. each layer needs progressively more
// banked accumulation, with a little gap below for the hysteresis (a layer
// that's already present survives down to roughly the previous threshold).
// tuned so a typical front leaves a layer or two, a long blizzard maxes out.
static const float SNOW_THRESH[WEATHERSIM_SNOW_MAX] = {
    1.0f, 2.4f, 4.2f, 6.5f
};

weathersim_ground weathersim_accum_ground(const weathersim_field *f,
                                          const weathersim_params *params,
                                          int gx, int gz, float column_y) {
    const weathersim_cell *c = weathersim_field_at_const(f, gx, gz);

    weathersim_ground g;
    g.accum = c->accum;
    g.snow_layers = 0;
    g.wetness = 0.0f;
    g.frozen = 0;

    // altitude-corrected temperature: the snow line climbs the mountainside.
    float alt = column_y - (float)WORLD_SEA_LEVEL;
    float temp_here = c->temp - alt * WEATHERSIM_LAPSE;

    // cold enough to hold snow? if so, count layers off the banked accum.
    if (temp_here <= params->snow_temp + 0.5f) {
        for (int i = 0; i < WEATHERSIM_SNOW_MAX; ++i) {
            if (c->accum >= SNOW_THRESH[i]) g.snow_layers = i + 1;
            else break;
        }
        // standing water freezes a touch below the snow threshold so you get
        // ice on ponds before the ground goes white.
        g.frozen = (temp_here <= params->snow_temp - 0.5f);
    }

    // wetness: above freezing the accum reads as puddle/damp instead of snow.
    // saturate it softly so a drizzle dampens but doesn't look flooded.
    if (g.snow_layers == 0) {
        float w = c->accum * 0.5f;
        g.wetness = w > 1.0f ? 1.0f : w;
    } else {
        // melting edge: snowy columns near the threshold are a bit slushy.
        g.wetness = 0.15f;
    }
    return g;
}

int weathersim_accum_snow_layers(const weathersim_field *f,
                                 const weathersim_params *params,
                                 vec3 world_pos) {
    float b = (float)WEATHERSIM_BLOCKS_PER_CELL;
    int gx = (int)floorf(world_pos.x / b) - f->origin_cx;
    int gz = (int)floorf(world_pos.z / b) - f->origin_cz;
    if (gx < 0) gx = 0; else if (gx >= WEATHERSIM_DIM) gx = WEATHERSIM_DIM - 1;
    if (gz < 0) gz = 0; else if (gz >= WEATHERSIM_DIM) gz = WEATHERSIM_DIM - 1;

    weathersim_ground g = weathersim_accum_ground(f, params, gx, gz,
                                                  world_pos.y);
    return g.snow_layers;
}
