#include "weathersim_precip.h"
#include <math.h>
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
if (over < 0.06f) return WEATHERSIM_PRECIP_DRIZZLE;
if (over < 0.18f) return WEATHERSIM_PRECIP_RAIN;
return WEATHERSIM_PRECIP_DOWNPOUR;
float sat = weathersim_precip_saturation(c->temp, params->dew_bias);
float over = c->humidity - sat - params->precip_threshold;
weathersim_precip kind = weathersim_precip_classify(c->temp, over,
                                                        params->snow_temp);
}
