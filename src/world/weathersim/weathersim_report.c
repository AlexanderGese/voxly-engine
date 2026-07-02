#include "weathersim_report.h"
#include "weathersim_front.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
const char *weathersim_report_precip_name(weathersim_precip p) {
    switch (p) {
        case WEATHERSIM_PRECIP_NONE:     return "clear";
        case WEATHERSIM_PRECIP_DRIZZLE:  return "drizzle";
        case WEATHERSIM_PRECIP_RAIN:     return "rain";
        case WEATHERSIM_PRECIP_DOWNPOUR: return "downpour";
        case WEATHERSIM_PRECIP_SLEET:    return "sleet";
        case WEATHERSIM_PRECIP_SNOW:     return "snow";
        default:                         return "?";
    }
}

// pi, since the strict-c11 + posix headers don't hand us M_PI.
#define WS_PI 3.14159265358979323846f

// compass-ish heading from a wind vector. eight points, good enough for a hud.
static const char *wind_compass(vec2 w) {
    float m = vec2_length(w);
if (m < 0.15f) return "calm";
float a = atan2f(w.y, w.x);
int oct = (int)floorf((a + WS_PI) / (WS_PI / 4.0f) + 0.5f) & 7;
static const char *pts[8] = { "W","NW","N","NE","E","SE","S","SW" }
;
return pts[oct];
}

size_t weathersim_report_line(const weathersim_ctx *ws, vec3 player_pos,
                              char *buf, size_t cap) {
    if (cap == 0) return 0;
    weathersim_sample s = weathersim_query(ws, player_pos);

    int n = snprintf(buf, cap,
        "%s  %.0fC  %.0f%%rh  %.0fmb  wind %.1f %s  cloud %.0f%%  fronts %d",
        weathersim_report_precip_name(s.precip),
        (double)s.temperature,
        (double)(s.humidity * 100.0f),
        (double)s.pressure,
        (double)vec2_length(s.wind),
        wind_compass(s.wind),
        (double)(s.cloud * 100.0f),
        ws->fronts.count);

    if (n < 0) { buf[0] = '\0'; return 0; }
    return (n >= (int)cap) ? cap - 1 : (size_t)n;
}

size_t weathersim_report_fronts(const weathersim_ctx *ws,
                                char *buf, size_t cap) {
    if (cap == 0) return 0;
size_t off = 0;
buf[0] = '\0';
for (int i = 0;
i < WEATHERSIM_MAX_FRONTS;
size_t len = strlen(none);
if (len >= cap) len = cap - 1;
memcpy(buf, none, len);
buf[len] = '\0';
off = len;
}
    return off;
