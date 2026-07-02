#include "weathersim_precip.h"

#include <math.h>

// saturation as a relative-humidity threshold. warmer air holds more moisture,
// so the *relative* threshold for condensation actually drops with temp in this
// simple model: cold air saturates easily (winter fog), warm air has headroom.
// it's a soft logistic centred on a mild temperature. not physically rigorous,
// but monotone and bounded, which is all the precip logic needs.
float weathersim_precip_saturation(float temp_c, float dew_bias) {
    float t = (temp_c - 8.0f) / WEATHERSIM_DEW_SLOPE;
    float s = 0.55f + 0.30f / (1.0f + expf(-t)); // ~0.55..0.85
    s += dew_bias;
    if (s < 0.2f) s = 0.2f;
    if (s > 0.98f) s = 0.98f;
    return s;
}

weathersim_precip weathersim_precip_classify(float temp_c, float over,
                                             float snow_temp) {
    if (over <= 0.0f) return WEATHERSIM_PRECIP_NONE;
    if (temp_c <= snow_temp) return WEATHERSIM_PRECIP_SNOW;
    if (temp_c <= snow_temp + 2.0f) return WEATHERSIM_PRECIP_SLEET;
    // above freezing: scale the rain band by how hard it's coming down.
    if (over < 0.06f) return WEATHERSIM_PRECIP_DRIZZLE;
    if (over < 0.18f) return WEATHERSIM_PRECIP_RAIN;
    return WEATHERSIM_PRECIP_DOWNPOUR;
}

float weathersim_precip_step(weathersim_field *f,
                             const weathersim_front_pool *fronts,
                             const weathersim_params *params, float dt) {
    int raining = 0;

    for (int gz = 0; gz < WEATHERSIM_DIM; ++gz) {
        for (int gx = 0; gx < WEATHERSIM_DIM; ++gx) {
            weathersim_cell *c = &f->cells[weathersim_field_idx(gx, gz)];

            float sat = weathersim_precip_saturation(c->temp, params->dew_bias);
            float over = c->humidity - sat - params->precip_threshold;

            // front weight gates and amplifies: thermodynamics alone gives
            // patchy fog, the fronts are what actually open the taps.
            float fw = weathersim_front_precip_weight(fronts, f, gx, gz);

            if (over > 0.0f && fw > 0.05f) {
                // condense a slice of the excess this tick. the front weight
                // makes storm cores rain harder. clamp the rate so a single
                // tick can't empty the column.
                float rate = over * (0.4f + 0.6f * fw);
                float fall = rate * dt;
                if (fall > over) fall = over;
                if (fall < 0.0f) fall = 0.0f;

                c->humidity -= fall;          // rain dries the air
                if (c->humidity < 0.0f) c->humidity = 0.0f;

                // bank it as accumulation. snow piles faster (lower density of
                // air moisture but it doesn't run off), rain partly runs off.
                float pile = (c->temp <= params->snow_temp) ? fall * 1.6f
                                                            : fall * 1.0f;
                c->accum += pile;
                if (c->accum > 8.0f) c->accum = 8.0f; // cap, it's not a flood sim
                ++raining;
            }

            // standing accumulation bleeds off: rain evaporates / drains, snow
            // melts slower. this is what makes the ground stay wet for a bit
            // after the front passes instead of snapping dry.
            float drain = params->accum_drain;
            if (c->temp <= params->snow_temp) drain *= 0.35f; // snow lingers
            c->accum -= drain * dt;
            if (c->accum < 0.0f) c->accum = 0.0f;

            // cloud cover tracks humidity with hysteresis: clouds build past
            // 0.6 and don't clear until humidity drops under 0.45.
            float target = 0.0f;
            if (c->humidity > 0.6f) target = (c->humidity - 0.6f) / 0.4f;
            else if (c->humidity > 0.45f && c->cloud > 0.1f)
                target = (c->humidity - 0.45f) / 0.55f;
            if (target > 1.0f) target = 1.0f;
            c->cloud += (target - c->cloud) * fminf(1.0f, 0.5f * dt);
        }
    }
    return (float)raining / (float)WEATHERSIM_CELLS;
}

weathersim_precip weathersim_precip_at(const weathersim_field *f,
                                       const weathersim_params *params,
                                       int gx, int gz, float *intensity_out) {
    const weathersim_cell *c = weathersim_field_at_const(f, gx, gz);
    float sat = weathersim_precip_saturation(c->temp, params->dew_bias);
    float over = c->humidity - sat - params->precip_threshold;

    weathersim_precip kind = weathersim_precip_classify(c->temp, over,
                                                        params->snow_temp);
    if (intensity_out) {
        // map over-saturation onto a 0..1 intensity for the particle budget.
        float it = over * 5.0f;
        if (it < 0.0f) it = 0.0f;
        if (it > 1.0f) it = 1.0f;
        // a little cloud floor so even sub-threshold overcast reads as gloomy.
        *intensity_out = fmaxf(it, kind == WEATHERSIM_PRECIP_NONE ? 0.0f
                                                                  : 0.15f);
    }
    return kind;
}
