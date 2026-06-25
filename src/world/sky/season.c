#include "season.h"
#include "sky_math.h"

#include <math.h>

voxl_sky_season voxl_sky_season_at(long day, int days_per_season) {
    if (days_per_season < 1) days_per_season = 1;

    long cycle = (long)days_per_season * VOXL_SEASON_COUNT;
    long d = day % cycle;
    if (d < 0) d += cycle;

    voxl_sky_season s;
    s.id = (voxl_sky_season_id)(d / days_per_season);
    s.progress = (float)(d % days_per_season) / (float)days_per_season;

    // a smooth temperature wave over the whole year, peaking mid-summer.
    // year phase 0..1, summer ~0.375 of the way (spring->summer).
    float year = (float)d / (float)cycle;
    s.temperature = sinf((year - 0.125f) * VOXL_SKY_TAU);   // -1..1

    // colder -> more snow. map temp [-1,1] to bias [1,0].
    s.snow_bias = voxl_sky_clampf((1.0f - s.temperature) * 0.5f, 0.0f, 1.0f);

    // daylight: long in summer, short in winter.
    s.day_length = voxl_sky_lerpf(8.0f, 16.0f, (s.temperature + 1.0f) * 0.5f);
    return s;
}

const char *voxl_sky_season_name(voxl_sky_season_id s) {
    switch (s) {
    case VOXL_SEASON_SPRING: return "spring";
    case VOXL_SEASON_SUMMER: return "summer";
    case VOXL_SEASON_AUTUMN: return "autumn";
    case VOXL_SEASON_WINTER: return "winter";
    default:                 return "?";
    }
}
